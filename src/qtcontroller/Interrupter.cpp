#include "tinia/qtcontroller/moc/Interrupter.hpp"

namespace tinia {
namespace qtcontroller {
namespace impl {

Interrupter::Interrupter(QObject *parent) :
    QObject(parent),
    m_job( NULL )
{
    connect( this, SIGNAL(interruptSignal()),
             this, SLOT( interruptSlot() ) );
}

void
Interrupter::setJob( jobcontroller::Job* job )
{
    m_job = job;
}

void
Interrupter::emitInterruptSignal()
{
    emit interruptSignal();
}


void
Interrupter::interruptSlot()
{
    if( m_job != NULL ) {
        m_job->handleInterrupt();
    }
}

} // namespace impl
} // namespace qtcontroller
} // namespace tinia
