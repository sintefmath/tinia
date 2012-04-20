#pragma once

#include <QObject>
#include <QWidget>
#include <string>
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/StateListener.hpp"

namespace tinia {
namespace qtobserver {

/** Controller object that lets a widget's visibilty follow a policy element.
  *
  * This object attaches itself to the widget, and continues to follow the
  * widget's life cycle.
  */
class VisibilityController : public QObject, public policy::StateListener
{
    Q_OBJECT;
public:
    explicit
    VisibilityController( QWidget*                               widget,
                          std::shared_ptr<policy::Policy>  policy,
                          const std::string&                     key,
                          const bool                             inverted );

    ~VisibilityController();

    void
    stateElementModified(policy::StateElement *stateElement);

signals:
    void
    setWidgetVisible( bool visibility );

protected:
    std::shared_ptr<policy::Policy>   m_policy;
    const std::string                       m_key;
    const bool                              m_inverted;
};

} // of namespace qtobserver
} // of namespace tinia
