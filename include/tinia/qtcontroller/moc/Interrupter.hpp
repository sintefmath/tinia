#ifndef TINIA_QTCONTROLLER_IMPL_INTERRUPTER_HPP
#define TINIA_QTCONTROLLER_IMPL_INTERRUPTER_HPP

#include <QObject>
#include "tinia/jobcontroller/Job.hpp"

namespace tinia {

namespace qtcontroller {
namespace impl {

class Interrupter : public QObject
{
    Q_OBJECT
public:
    explicit Interrupter(QObject *parent = 0);

    void
    setJob( jobcontroller::Job* job );

    void
    emitInterruptSignal();

signals:
    void
    interruptSignal();


public slots:
    void
    interruptSlot();

private:
    jobcontroller::Job*     m_job;
};

} // namespace impl
} // namespace qtcontroller
} // namespace tinia

#endif // TINIA_QTCONTROLLER_IMPL_INTERRUPTER_HPP
