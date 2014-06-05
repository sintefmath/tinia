#include <QtGlobal>
#include <QThread>
#include <QMutexLocker>
#include "tinia/qtcontroller/moc/Invoker.hpp"
namespace tinia {
namespace qtcontroller {
namespace impl {

Invoker::Invoker( QObject* parent )
    : QObject( parent ),
      m_main_thread( QThread::currentThread() )
{
    connect( this, SIGNAL( requestMainThreadInvocation(QRunnable*,bool*)),
             this, SLOT( handleMainThreadInvocation(QRunnable*,bool*)) );
}

void
Invoker::invokeInMainThread( QRunnable* function, bool block )
{
    Q_ASSERT( function != NULL );
    if( !block ) {
        emit requestMainThreadInvocation( function, NULL );
    }
    else {
        bool i_am_done = false; // specific condition for this request.
        emit requestMainThreadInvocation( function, &i_am_done );

        // wait for function to complete
        QMutexLocker locker( &m_wait_mutex );
        while ( !i_am_done ) {
            m_wait_condition.wait( &m_wait_mutex );
        }
    }
}


void
Invoker::handleMainThreadInvocation( QRunnable* function, bool* i_am_done )
{
    Q_ASSERT( QThread::currentThread() == m_main_thread );
    function->run();

    // notify completion if invocation is blocking.
    if( i_am_done != NULL ) {
        QMutexLocker locker( &m_wait_mutex );
        *i_am_done = true;
        m_wait_condition.wakeAll();
    }
}



} // namespace impl
} // namespace qtcontroller
} // namespace tinia
