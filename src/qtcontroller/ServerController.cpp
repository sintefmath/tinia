#include "tinia/qtcontroller/moc/ServerController.hpp"

namespace tinia {
namespace qtcontroller {
namespace impl {

ServerController::ServerController(tinia::jobcontroller::Job* job,
                                   QObject *parent) :
    QObject(parent), m_job(job), m_server(NULL)
{
}

void ServerController::startServer(bool start)
{
    if(start) {
        m_server = new HTTPServer(m_job, this);
    }
    else if(m_server != NULL) {
        delete m_server;
        m_server = NULL;
    }
}

} // namespace impl
} // namespace qtcontroller
} // namespace tinia
