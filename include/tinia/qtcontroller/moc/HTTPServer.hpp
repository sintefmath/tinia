#pragma once

#include "tinia/jobcontroller.hpp"
#include "tinia/qtcontroller/moc/OpenGLServerGrabber.hpp"
#include "tinia/model/impl/xml/XMLHandler.hpp"
#include <QTcpServer>
#include <QTcpSocket>

namespace tinia {
namespace qtcontroller {
namespace impl {

class HTTPServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit HTTPServer(tinia::jobcontroller::Job*,
                        QObject *parent = 0);

    void incomingConnection(int socket);
    

    tinia::jobcontroller::Job* m_job;

    tinia::qtcontroller::impl::OpenGLServerGrabber m_serverGrabber;

};

}
} // namespace qtcontroller
} // namespace tinia

