#pragma once

#include <QObject>
#include <QWidget>
#include <string>
#include "tinia/policylib/PolicyLib.hpp"
#include "tinia/policylib/StateListener.hpp"

namespace qtobserver {

class EnabledController : public QObject, public policylib::StateListener
{
    Q_OBJECT;
public:
    explicit
    EnabledController( QWidget*                               widget,
                       std::shared_ptr<policylib::PolicyLib>  policylib,
                       const std::string&                     key,
                       const bool                             inverted );

    ~EnabledController();

    void
    stateElementModified(policylib::StateElement *stateElement);

signals:
    void
    setWidgetEnabled( bool enabled );

protected:
    std::shared_ptr<policylib::PolicyLib>   m_policylib;
    const std::string                       m_key;
    const bool                              m_inverted;
};



} // of namespace qtobserver
