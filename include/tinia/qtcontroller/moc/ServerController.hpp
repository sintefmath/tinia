#pragma once

#include <QObject>
#include <tinia/jobcontroller.hpp>
#include <tinia/qtcontroller/moc/HTTPServer.hpp>

namespace tinia {
namespace qtcontroller {
namespace impl {

class ServerController : public QObject
{
    Q_OBJECT
public:
    explicit ServerController(tinia::jobcontroller::Job* job,
                              QObject *parent = 0);
    
signals:
    
public slots:
    void startServer(bool start);

private:
    tinia::jobcontroller::Job* m_job;
    impl::HTTPServer* m_server; // Lifetime managed by qt parent-child machinery.
    
};

} // namespace impl
} // namespace qtcontroller
} // namespace tinia

