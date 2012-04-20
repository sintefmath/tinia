#pragma once

#include <QObject>
#include <QWidget>
#include <string>
#include "tinia/policylib/PolicyLib.hpp"
#include "tinia/policylib/StateListener.hpp"

namespace qtobserver {

/** Controller object that lets a widget's visibilty follow a policylib element.
  *
  * This object attaches itself to the widget, and continues to follow the
  * widget's life cycle.
  */
class VisibilityController : public QObject, public policylib::StateListener
{
    Q_OBJECT;
public:
    explicit
    VisibilityController( QWidget*                               widget,
                          std::shared_ptr<policylib::PolicyLib>  policylib,
                          const std::string&                     key,
                          const bool                             inverted );

    ~VisibilityController();

    void
    stateElementModified(policylib::StateElement *stateElement);

signals:
    void
    setWidgetVisible( bool visibility );

protected:
    std::shared_ptr<policylib::PolicyLib>   m_policylib;
    const std::string                       m_key;
    const bool                              m_inverted;
};

} // of namespace qtobserver
