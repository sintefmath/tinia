
#include "tinia/qtobserver/moc/EnabledController.hpp"

namespace tinia {
namespace qtobserver {

EnabledController::EnabledController( QWidget*                               widget,
                                      std::shared_ptr<policy::Policy>  policy,
                                      const std::string&                     key,
                                      const bool                             inverted )
    : QObject( widget ),
      m_policy( policy ),
      m_key( key ),
      m_inverted( inverted )
{
    // We set the widget as the parent, and qt should delete this object when
    // widget is deleted. Otherwise, we could do connect the widget's destroyed
    // signal to this object deleteLater slot (but this shouldn't be necessary)
    connect( this, SIGNAL(setWidgetEnabled(bool)), widget, SLOT(setEnabled(bool)) );

    m_policy->addStateListener( m_key, this );

    bool value;
    m_policy->getElementValue( m_key, value );

    bool enabled = (value && !m_inverted ) || (!value && m_inverted );
    widget->setEnabled( enabled );
}

EnabledController::~EnabledController()
{
    m_policy->removeStateListener( m_key, this );
}

void
EnabledController::stateElementModified(policy::StateElement *stateElement)
{
    bool value;
    stateElement->getValue( value );

    bool enabled = (value && !m_inverted ) || (!value && m_inverted );
    emit setWidgetEnabled( enabled );
}


} // of namespace qtobserver
} // of namespace tinia
