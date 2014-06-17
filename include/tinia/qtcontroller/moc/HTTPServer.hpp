#pragma once

#include "tinia/jobcontroller.hpp"
#include "tinia/qtcontroller/moc/OpenGLServerGrabber.hpp"
#include "tinia/qtcontroller/moc/Invoker.hpp"
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

    /**
     * Takes control over imageSource
     */
    explicit HTTPServer( tinia::jobcontroller::Job*,
        QObject *parent = 0);

    void incomingConnection(int socket);
    
private:
    tinia::jobcontroller::Job*  m_job;
    OpenGLServerGrabber*        m_serverGrabber;    // Lifetime managed by Qt child-parent
    Invoker*                    m_mainthread_invoker;   // Lifetime managed by Qt child-parent.

};

}
} // namespace qtcontroller
} // namespace tinia

