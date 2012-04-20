#pragma once

#include <QObject>
#include <QWidget>
#include <string>
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/StateListener.hpp"

namespace tinia {
namespace qtobserver {

class EnabledController : public QObject, public policy::StateListener
{
    Q_OBJECT;
public:
    explicit
    EnabledController( QWidget*                               widget,
                       std::shared_ptr<policy::Policy>  policy,
                       const std::string&                     key,
                       const bool                             inverted );

    ~EnabledController();

    void
    stateElementModified(policy::StateElement *stateElement);

signals:
    void
    setWidgetEnabled( bool enabled );

protected:
    std::shared_ptr<policy::Policy>   m_policy;
    const std::string                       m_key;
    const bool                              m_inverted;
};



} // of namespace qtobserver
} // of namespace tinia
