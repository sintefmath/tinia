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

    /**
     * Takes control over imageSource
     */
    explicit HTTPServer(tinia::jobcontroller::Job*,
        tinia::qtcontroller::ImageSource* imageSource,
        QObject *parent = 0);

    void incomingConnection(int socket);
    
private:
    tinia::jobcontroller::Job* m_job;

    boost::scoped_ptr<tinia::qtcontroller::ImageSource> m_serverGrabber;

};

}
} // namespace qtcontroller
} // namespace tinia

