#include "tinia/qtcontroller/moc/ServerThread.hpp"
#include <QTcpSocket>
#include "tinia/qtcontroller/impl/http_utils.hpp"
#include "GL/glew.h"
#include <QImage>
#include <QBuffer>
#include "tinia/renderlist.hpp"
#include <QFile>
#include "tinia/qtcontroller/moc/LongPollHandler.hpp"

namespace tinia {
namespace qtcontroller {
namespace impl {

ServerThread::ServerThread(OpenGLServerGrabber& grabber,
                           tinia::jobcontroller::Job* job,
                           int socket,
                           QObject *parent) :
    QThread(parent), m_socket(socket), m_job(job),
    m_xmlHandler(job->getExposedModel()),
    m_grabber(grabber)
{
}

void ServerThread::run()
{
    qDebug("before create");
    QTcpSocket socket;
    qDebug("after create");

    socket.setSocketDescriptor(m_socket);
    socket.waitForReadyRead();


    qDebug("ServerThread: receiving");
    if (socket.canReadLine()) {
        auto request = socket.readAll();
        QTextStream os(&socket);
        if(isLongPoll(request)) {
            LongPollHandler handler(os, request, m_job->getExposedModel());
            handler.handle();
        }
        else if (isGetOrPost(request)) {
            os.setAutoDetectUnicode(true);
            if(!handleNonStatic(os, getRequestURI(request), request)) {
                os << getStaticContent(getRequestURI(request)) << "\n";
            }

           // socket.disconnectFromHost();

        }
        socket.close();
        socket.waitForDisconnected();
    }
}

void ServerThread::discardClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();
}

bool ServerThread::isLongPoll(const QString &request)
{
    return getRequestURI(request) == "/getExposedModelUpdate.xml";
}


void ServerThread::getSnapshotTxt(QTextStream &os, const QString &request)
{
    auto arguments =
            parseGet<boost::tuple<unsigned int, unsigned int,
            std::string> >(decodeGetParameters(request), "width height key");

    auto width = arguments.get<0>();
    auto height = arguments.get<1>();
    auto key = arguments.get<2>();

    m_grabber.getImageAsText(os, width, height, QString(key.c_str()));
}
bool ServerThread::handleNonStatic(QTextStream &os, const QString& file,
                                 const QString& request)
{
    std::cout << file.toStdString() << std::endl;
    std::cout << "======================"<<std::endl;
    std::cout << request.toStdString()<< std::endl;
    std::cout << "=======================" <<std::endl;
    try {
        if(file == "/snapshot.txt") {

            updateState(os, request);
            getSnapshotTxt(os, request);
            return true;
        }
        else if(file == "/getRenderList.xml") {
            getRenderList(os, request);
            return true;
        }
        /*else if(file == "/getExposedModelUpdate.xml") {
            getPolicyUpdate(os, request);
            return true;
        }*/
        else if(file =="/updateState.xml") {
            updateState(os, request);
            os << httpHeader("application/xml")<<"\n";
            return true;
        }

    } catch(std::invalid_argument& e) {
        errorCode(os, 400, e.what());
        return true;
    }

    return false;
}


void ServerThread::updateState(QTextStream &os, const QString &request)
{
    std::string content = getPostContent(request).toStdString();
    m_xmlHandler.updateState(content.c_str(), content.size());
}

void ServerThread::getRenderList(QTextStream &os, const QString &request)
{
    auto params = parseGet<boost::tuple<std::string, unsigned int> > (decodeGetParameters(request), "key timestamp");
    os << httpHeader("application/xml") << "\n";
    tinia::jobcontroller::OpenGLJob* openglJob = dynamic_cast<tinia::jobcontroller::OpenGLJob*>(m_job);
    if(openglJob) {
        auto db = openglJob->getRenderList("session", params.get<0>());
        if(db) {
            std::string list = renderlist::getUpdateXML( db,
                                                         renderlist::ENCODING_JSON,
                                                         params.get<1>() );
            os << QString(list.c_str()) << "\n";
        }

    }
}

void ServerThread::errorCode(QTextStream &os, unsigned int code, const QString &msg)
{
    os << "HTTP/1.1 " << QString::number(code) << "\r\n"
       << "Content-Type: text/html; charset=\"utf-8\"\r\n"
       << "<html>\n<head>\n<title>Error: " << QString::number(code) << "</title>"
       << "</head>\n" <<"<body>\n" << "<h1>Error code: " << code << "</h1>\n"<<msg
       <<"</body>\n"<<"</html>\n";
}


QString ServerThread::getStaticContent(const QString &uri)
{

    QString fullPath = ":/javascript" + uri;

    QFile file(fullPath);
    if(file.open(QIODevice::ReadOnly)) {
        QString reply =QString("HTTP/1.0 200 Ok\r\n") +
                QString("Content-Type: ") + getMimeType(uri) +
                QString("; charset=\"utf-8\"\r\n \n\n") + file.readAll() + "\n";
        return reply;
    }
    else {
        return "HTTP/1.0 404 Not Found\r\n";
    }
}

} // namespace impl
} // namespace qtcontroller
} // namespace tinia