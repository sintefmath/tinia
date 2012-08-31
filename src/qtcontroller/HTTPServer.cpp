/* Copyright STIFTELSEN SINTEF 2012
 *
 * This file is part of the Tinia Framework.
 *
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QImage>
#include <QBuffer>
#include <QByteArray>
#include <GL/glew.h>
#include "tinia/qtcontroller/moc/HTTPServer.hpp"
#include "tinia/qtcontroller/impl/http_utils.hpp"
#include "tinia/renderlist.hpp"
#include <stdexcept>
#include <iostream>
#include <QStringList>
#include <QDateTime>
#include <QFile>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>

namespace tinia {
namespace qtcontroller {
namespace impl {

HTTPServer::HTTPServer(tinia::jobcontroller::Job* job, QObject *parent) :
    QTcpServer(parent), m_xmlHandler(job->getExposedModel()), m_job(job),
    m_openglIsReady(false)
{
    listen(QHostAddress::Any, 8080);
    qDebug("Started");

}

HTTPServer::~HTTPServer()
{
    if(m_openglIsReady) {
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteRenderbuffers(1, &m_renderbufferRGBA);
        glDeleteRenderbuffers(1, &m_renderbufferDepth);
    }
}

void HTTPServer::incomingConnection(int socket)
{
    qDebug("HTTPServer: request");

    QTcpSocket* s = new QTcpSocket(this);
    connect(s, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
    s->setSocketDescriptor(socket);

}

void HTTPServer::readyRead()
{
    qDebug("HTTPServer: receiving");
    QTcpSocket* socket = (QTcpSocket*)sender();
    if (socket->canReadLine()) {
        auto request = socket->readAll();
        if (isGetOrPost(request)) {
            QTextStream os(socket);
            os.setAutoDetectUnicode(true);
            if(!handleNonStatic(os, getRequestURI(request), request)) {
                os << getStaticContent(getRequestURI(request)) << "\n";
            }

            socket->close();


            if (socket->state() == QTcpSocket::UnconnectedState) {
                delete socket;
            }
        }
    }

}

void HTTPServer::discardClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();

}

void HTTPServer::setupOpenGL()
{
    glewInit();
    glGenFramebuffers( 1, &m_fbo );
    glGenRenderbuffers( 1, &m_renderbufferRGBA );
    glGenRenderbuffers( 1, &m_renderbufferDepth );
    m_openglIsReady = true;
}

void HTTPServer::getSnapshotTxt(QTextStream &os, const QString &request)
{
    auto openGLJob = static_cast<tinia::jobcontroller::OpenGLJob*>(m_job);
    if(!openGLJob) {
        throw std::invalid_argument("This is not an OpenGL job!");
    }

    auto arguments =
            parseGet<boost::tuple<unsigned int, unsigned int,
            std::string> >(decodeGetParameters(request), "width height key");
    if(!m_openglIsReady) {
        setupOpenGL();
    }

    auto width = arguments.get<0>();
    auto height = arguments.get<1>();
    auto key = arguments.get<2>();
    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
    glBindRenderbuffer( GL_RENDERBUFFER, m_renderbufferRGBA );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA, width, height );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_RENDERBUFFER,
                               m_renderbufferRGBA );

    glBindRenderbuffer( GL_RENDERBUFFER, m_renderbufferDepth );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                               GL_DEPTH_ATTACHMENT,
                               GL_RENDERBUFFER,
                               m_renderbufferDepth );

    glViewport( 0, 0, width, height );



    openGLJob->renderFrame( "session", key, m_fbo, width, height );

     glPixelStorei( GL_PACK_ALIGNMENT, 1 );
     unsigned char buffer[5000000];
     glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer );
     QImage img(buffer, width, height, QImage::Format_RGB888);
     QBuffer qBuffer;

     img.save(&qBuffer, "png");
     os << httpHeader(getMimeType("file.txt"));
     QString str(QByteArray(qBuffer.data(), int(qBuffer.size())).toBase64());
     os << "\n"<<str;

}

bool HTTPServer::handleNonStatic(QTextStream &os, const QString& file,
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
        else if(file == "/getExposedModelUpdate.xml") {
            getPolicyUpdate(os, request);
            return true;
        }
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

void HTTPServer::getPolicyUpdate(QTextStream &os, const QString &request)
{
    std::cout <<"Getting policyupdate" << std::endl;
    try {
        auto params = parseGet<boost::tuple<unsigned int> >(decodeGetParameters(request), "revision");
        char buffer[5000000];
        m_xmlHandler.getExposedModelUpdate(buffer, 5000000, params.get<0>());
        os << httpHeader("application/xml")<<"\n";
        os << QString(buffer)<< "\n";
    } catch(std::invalid_argument e) {
        char buffer[5000000];
        m_xmlHandler.getExposedModelUpdate(buffer, 5000000, 0);
        os << httpHeader("application/xml")<<"\n";
        os << QString(buffer)<< "\n";
    }
}

void HTTPServer::updateState(QTextStream &os, const QString &request)
{
    std::string content = getPostContent(request).toStdString();
    m_xmlHandler.updateState(content.c_str(), content.size());
}

void HTTPServer::getRenderList(QTextStream &os, const QString &request)
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

void HTTPServer::errorCode(QTextStream &os, unsigned int code, const QString &msg)
{
    os << "HTTP/1.1 " << QString::number(code) << "\r\n"
       << "Content-Type: text/html; charset=\"utf-8\"\r\n"
       << "<html>\n<head>\n<title>Error: " << QString::number(code) << "</title>"
       << "</head>\n" <<"<body>\n" << "<h1>Error code: " << code << "</h1>\n"<<msg
       <<"</body>\n"<<"</html>\n";
}


QString HTTPServer::getStaticContent(const QString &uri)
{
    QFile file(":/javascript" + uri);
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

}
} // namespace qtcontroller
} // namespace tinia
