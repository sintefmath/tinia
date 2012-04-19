#include "qtobserver/moc/VisibilityController.hpp"

namespace qtobserver {

VisibilityController::VisibilityController( QWidget*                               widget,
                                            std::shared_ptr<policylib::PolicyLib>  policylib,
                                            const std::string&                     key,
                                            const bool                             inverted )
    : QObject( widget ),
      m_policylib( policylib ),
      m_key( key ),
      m_inverted( inverted )
{
    // We set the widget as the parent, and qt should delete this object when
    // widget is deleted. Otherwise, we could do connect the widget's destroyed
    // signal to this object deleteLater slot (but this shouldn't be necessary)
    connect( this, SIGNAL(setWidgetVisible(bool)), widget, SLOT(setVisible(bool)) );

    m_policylib->addStateListener( m_key, this );

    bool value;
    m_policylib->getElementValue( m_key, value );

    bool visible = (value && !m_inverted ) || (!value && m_inverted );
    widget->setVisible( visible );
}

VisibilityController::~VisibilityController()
{
    m_policylib->removeStateListener( m_key, this );
}

void
VisibilityController::stateElementModified(policylib::StateElement *stateElement)
{
    bool value;
    stateElement->getValue( value );

    bool visible = (value && !m_inverted ) || (!value && m_inverted );
    emit setWidgetVisible( visible );
}


} // of namespace qtobserver
