#include "tinia/qtobserver/moc/StringController.hpp"
#include <QLineEdit>
#include <QLabel>
#include <QAbstractButton>

namespace tinia {
namespace qtobserver {

StringController::StringController( QWidget *widget,
                                    std::shared_ptr<policy::Policy>   policy,
                                    const std::string&                      key,
                                    const bool                              show_value,
                                    const QString&                          suffix )
    : QObject( widget ),
      m_policy( policy ),
      m_key( key ),
      m_show_value( show_value ),
      m_suffix( suffix ),
      m_button( NULL )
{
    if( m_show_value ) {
        m_current_value = m_policy->getElementValueAsString( m_key );
    }
    else {
        policy::StateSchemaElement element = m_policy->getStateSchemaElement(key);
        if( element.emptyAnnotation() ) {
            m_current_value = m_key;
        }
        else {
            auto annotations = element.getAnnotation();
            auto annotation = annotations["en"];
            if( annotation.empty() ) {
                m_current_value = m_key;
            }
            else {
                m_current_value = annotation;
            }
        }
    }

    QLineEdit* line_edit = dynamic_cast<QLineEdit*>( widget );
    if( line_edit != NULL ) {
        line_edit->setText(  QString( m_current_value.c_str() + m_suffix ) );
        connect( line_edit, SIGNAL(textChanged(QString)),
                 this, SLOT(textChangeFromQt(QString)) );
        connect( this, SIGNAL( textChangeFromPolicy(QString)),
                line_edit, SLOT(setText(QString)) );
    }
    QLabel* label = dynamic_cast<QLabel*>( widget );
    if( label != NULL ) {
        label->setText(  QString( m_current_value.c_str() + m_suffix ) );
        connect( this, SIGNAL( textChangeFromPolicy(QString)),
                 label, SLOT( setText(QString)) );
    }
    QAbstractButton* button = dynamic_cast<QAbstractButton*>( widget );
    if( button != NULL ) {
        m_button = button;
        m_button->setText( QString( m_current_value.c_str() + m_suffix ) );
    }

    if( m_show_value ) {
        m_policy->addStateListener( m_key, this );
    }

}

StringController::~StringController()
{
    if( m_show_value ) {
        m_policy->removeStateListener( m_key, this );
    }
}

void
StringController::stateElementModified( policy::StateElement *state_element )
{
    const std::string& value = state_element->getStringValue();
    if( value != m_current_value ) {
        m_current_value = value;
        if( m_button != NULL ) {
            // button doesn't have an appropriate slot
            m_button->setText( QString( m_current_value.c_str() + m_suffix ) );
        }
        else {
            emit textChangeFromPolicy( QString( m_current_value.c_str() + m_suffix ) );
        }
    }
}


void
StringController::textChangeFromQt( const QString& text )
{
    if( m_suffix.size() != 0 ) {
        // sanity check. When we add sufficies, don't propagate back to
        // policy.
        return;
    }
    if( m_show_value ) {
      const std::string value( text.toLocal8Bit() );
        if( m_current_value != value ) {
			try {
				m_policy->updateElementFromString( m_key, value );
				m_current_value = value;
			} catch (...) {
				if(value == "") {
					m_current_value = "";
				}
				emit textChangeFromPolicy( m_current_value.c_str() );
			}
        }
    }
    else {
        // we do not support editing of key annotations
        emit textChangeFromPolicy( m_current_value.c_str() );
    }
}


} // of namespace qtobserver
} // of namespace tinia
