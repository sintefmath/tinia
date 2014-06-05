#pragma once

#include <QObject>
#include <QMutex>
#include <QRunnable>
#include <QWaitCondition>

namespace tinia {
namespace qtcontroller {
namespace impl {


class Invoker : public QObject
{
    Q_OBJECT
public:


    /** Create a new invoker object.
     * \note Must be created in main thread.
     */
    explicit Invoker( QObject* parent );

    void
    invokeInMainThread( QRunnable* function, bool block = true );


signals:

    void
    requestMainThreadInvocation( QRunnable* function, bool* i_am_done );

private slots:

    void
    handleMainThreadInvocation( QRunnable* function, bool *i_am_done );

private:
    QThread*        m_main_thread;
    QMutex          m_wait_mutex;

    /** Shared wait conditions for all invocations.
     *
     * This wait condition is shared by all invocations, as the number of
     * concurrent invocations is currently assumed to be low. We use a bool
     * flag to specify which condition that is true.
     *
     * Refers to \ref m_wait_mutex.
     */
    QWaitCondition  m_wait_condition;

};

} // namespace impl
} // namespace qtcontroller
} // namespace tinia
