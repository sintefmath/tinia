#include "tinia/qtcontroller/impl/ServerThread.hpp"
#include <QTcpSocket>
#include "tinia/qtcontroller/impl/http_utils.hpp"
#include "GL/glew.h"
#include <QImage>
#include <QBuffer>
#include <QRegExp>
#include "tinia/renderlist.hpp"
#include <QFile>
#include <QMutexLocker>
#include "tinia/qtcontroller/moc/LongPollHandler.hpp"
#include "tinia/model/ExposedModelLock.hpp"

namespace {

class RenderListFetcher : public QRunnable
{
public:
    explicit RenderListFetcher( QTextStream& reply,
                                const QString& request,
                                tinia::jobcontroller::Job* job )
        : m_reply( reply ),
          m_request( request )
    {
        using namespace tinia::qtcontroller::impl;
        
        m_job = dynamic_cast<tinia::jobcontroller::OpenGLJob*>( job );
        if( m_job == NULL ) {
            throw std::invalid_argument("This is not an OpenGL job!");
        }

        m_params = parseGet<boost::tuple<std::string, unsigned int> >( decodeGetParameters(request),
                                                                       "key timestamp" );
    }
    
    ~RenderListFetcher()
    {
        using namespace tinia::qtcontroller::impl;

        m_reply << httpHeader("application/xml") << "\r\n";
        m_reply << m_update << "\n";
    }
    
    void
    run()
    {
        // runs as GUI thread
        using namespace tinia::renderlist;
        const DataBase* db = m_job->getRenderList( "session", m_params.get<0>() );
        if(db) {
            std::string list = getUpdateXML( db, ENCODING_JSON, m_params.get<1>() );
            m_update = QString( list.c_str() );
        }
    }
    
protected:
    QTextStream&                            m_reply;
    const QString&                          m_request;
    tinia::jobcontroller::OpenGLJob*        m_job;
    boost::tuple<std::string, unsigned int> m_params;
    QString                                 m_update;
};


class SnapshotAsTextFetcher : public QRunnable
{
public:
    
    explicit SnapshotAsTextFetcher( QTextStream& reply,
                              const QString& request,
                              tinia::jobcontroller::Job* job,
                              tinia::qtcontroller::impl::OpenGLServerGrabber* gl_grabber )
        : m_reply( reply ),
          m_request( request ),
          m_job( NULL ),
          m_gl_grabber( gl_grabber ),
          m_gl_grabber_locker( gl_grabber->exclusiveAccessMutex() )
    {
        using namespace tinia::qtcontroller::impl;
        
        m_job = dynamic_cast<tinia::jobcontroller::OpenGLJob*>( job );
        if( m_job == NULL ) {
            throw std::invalid_argument("This is not an OpenGL job!");
        }
        
        
        typedef boost::tuple<unsigned int, unsigned int, std::string> params_t;
        params_t arguments = parseGet<params_t >(decodeGetParameters(request),
                                                 "width height key" );
        m_width  = arguments.get<0>();
        m_height = arguments.get<1>();
        m_key    = arguments.get<2>();
    }
    
    ~SnapshotAsTextFetcher()
    {
        using namespace tinia::qtcontroller::impl;
        QImage img( m_gl_grabber->imageBuffer(),
                    m_width,
                    m_height,
                    QImage::Format_RGB888 );
    
        // This is a temporary fix. The image is reflected through the horizontal
        // line y=height ((x, y) |--> (x, h-y) ).
        QTransform flipTransformation(1, 0,
                                      0, -1,
                                      0, m_height);
        img = img.transformed(flipTransformation);
        QBuffer qBuffer;
        img.save(&qBuffer, "png");
        //m_gl_grabber_locker.unlock();
        
        m_reply << httpHeader(getMimeType("file.txt"));
        QString str( QByteArray( qBuffer.data(),
                                 int(qBuffer.size()) ).toBase64() );
        m_reply << "\r\n"<<str;        
    }
    
    void
    run()
    {
        m_gl_grabber->grab( m_job, m_width, m_height, m_key ); 
    }

    
    
protected:
    QTextStream&                                    m_reply;
    const QString&                                  m_request;
    tinia::jobcontroller::OpenGLJob*                m_job;
    tinia::qtcontroller::impl::OpenGLServerGrabber* m_gl_grabber;
    QMutexLocker                                    m_gl_grabber_locker;
    unsigned int                                    m_width;
    unsigned int                                    m_height;
    std::string                                     m_key;
};


}


namespace tinia {
namespace qtcontroller {
namespace impl {

ServerThread::ServerThread(OpenGLServerGrabber &grabber,
                           Invoker* mainthread_invoker,
                           tinia::jobcontroller::Job* job,
                           int socket,
                           QObject *parent) :
    m_socket(socket),
    m_xmlHandler(job->getExposedModel()),
    m_job(job),
    m_grabber(grabber),
    m_mainthread_invoker(mainthread_invoker)
{
}

void ServerThread::run()
{
    QTcpSocket socket;

    socket.setSocketDescriptor(m_socket);
    socket.waitForReadyRead();

    if (socket.canReadLine()) {

        QByteArray request = socket.readAll();
        while(!request.contains("\r\n\r\n")) {
            socket.waitForBytesWritten();
            request += socket.readAll();
        }
        //Now we have the whole header
        QRegExp contentLengthExpression("Content-Length: (\\d+)\\s*\\r\\n");

        
        if (contentLengthExpression.indexIn(request) != -1) {
            QString contentLengthGroup = contentLengthExpression.cap(1);
            int contentLength = contentLengthGroup.toInt();
            
            int headerSize = request.indexOf("\r\n\r\n") + 4;
            while(request.size()  - headerSize < contentLength) {
                socket.waitForBytesWritten();
                request += socket.readAll();
            }
        }
        QTextStream os(&socket);
        if(isLongPoll(request)) {
            LongPollHandler handler(os, request, m_job->getExposedModel());
            handler.handle();
        }
        else if (isGetOrPost(request)) {
            os.setAutoDetectUnicode(true);
            if(!handleNonStatic(os, getRequestURI(request), request)) {
                os << getStaticContent(getRequestURI(request)) << "\r\n";
            }

           // socket.disconnectFromHost();

        }
            

        socket.close();
        socket.waitForDisconnected();

    }
}

bool ServerThread::isLongPoll(const QString &request)
{
    return getRequestURI(request) == "/getExposedModelUpdate.xml";
}


void ServerThread::getSnapshotTxt(QTextStream &os, const QString &request)
{
    boost::tuple<unsigned int, unsigned int,
            std::string> arguments =
            parseGet<boost::tuple<unsigned int, unsigned int,
            std::string> >(decodeGetParameters(request), "width height key");

    unsigned int width = arguments.get<0>();
    unsigned int height = arguments.get<1>();
    std::string key = arguments.get<2>();

    m_grabber.getImageAsText(os, width, height, QString(key.c_str()));
}
bool ServerThread::handleNonStatic(QTextStream &os, const QString& file,
                                 const QString& request)
{
    try {
        if(file == "/snapshot.txt") {
            updateState(os, request);
            SnapshotAsTextFetcher f( os, request, m_job, &m_grabber );
            m_mainthread_invoker->invokeInMainThread( &f, true );
            //getSnapshotTxt(os, request);
            return true;
        }
        else if(file == "/getRenderList.xml") {
            RenderListFetcher f( os, request, m_job );
            m_mainthread_invoker->invokeInMainThread( &f, true );
            return true;
        }
        else if(file =="/updateState.xml") {
            updateState(os, request);
            os << httpHeader("application/xml") << "\r\n";
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
	tinia::model::ExposedModelLock lock(m_job->getExposedModel());
    std::string content = getPostContent(request).toStdString();
    if( !content.empty() ) {
        m_xmlHandler.updateState(content.c_str(), content.size());
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
        QString reply =QString("HTTP/1.1 200 Ok\r\n") +
                QString("Content-Type: ") + getMimeType(uri) +
                QString("; charset=\"utf-8\"\r\n\r\n\r\n") + file.readAll() + "\r\n";
        return reply;
    }
    else {
        return "HTTP/1.1 404 Not Found\r\n";
    }
}

} // namespace impl
} // namespace qtcontroller
} // namespace tinia
