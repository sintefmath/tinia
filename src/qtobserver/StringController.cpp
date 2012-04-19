#include "qtobserver/moc/StringController.hpp"
#include <QLineEdit>
#include <QLabel>
#include <QAbstractButton>

namespace qtobserver {

StringController::StringController( QWidget *widget,
                                    std::shared_ptr<policylib::PolicyLib>   policylib,
                                    const std::string&                      key,
                                    const bool                              show_value,
                                    const QString&                          suffix )
    : QObject( widget ),
      m_policylib( policylib ),
      m_key( key ),
      m_show_value( show_value ),
      m_suffix( suffix ),
      m_button( NULL )
{
    if( m_show_value ) {
        m_current_value = m_policylib->getElementValueAsString( m_key );
    }
    else {
        policylib::StateSchemaElement element = m_policylib->getStateSchemaElement(key);
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
        connect( this, SIGNAL( textChangeFromPolicyLib(QString)),
                line_edit, SLOT(setText(QString)) );
    }
    QLabel* label = dynamic_cast<QLabel*>( widget );
    if( label != NULL ) {
        label->setText(  QString( m_current_value.c_str() + m_suffix ) );
        connect( this, SIGNAL( textChangeFromPolicyLib(QString)),
                 label, SLOT( setText(QString)) );
    }
    QAbstractButton* button = dynamic_cast<QAbstractButton*>( widget );
    if( button != NULL ) {
        m_button = button;
        m_button->setText( QString( m_current_value.c_str() + m_suffix ) );
    }

    if( m_show_value ) {
        m_policylib->addStateListener( m_key, this );
    }

}

StringController::~StringController()
{
    if( m_show_value ) {
        m_policylib->removeStateListener( m_key, this );
    }
}

void
StringController::stateElementModified( policylib::StateElement *state_element )
{
    const std::string& value = state_element->getStringValue();
    if( value != m_current_value ) {
        m_current_value = value;
        if( m_button != NULL ) {
            // button doesn't have an appropriate slot
            m_button->setText( QString( m_current_value.c_str() + m_suffix ) );
        }
        else {
            emit textChangeFromPolicyLib( QString( m_current_value.c_str() + m_suffix ) );
        }
    }
}


void
StringController::textChangeFromQt( const QString& text )
{
    if( m_suffix.size() != 0 ) {
        // sanity check. When we add sufficies, don't propagate back to
        // policylib.
        return;
    }
    if( m_show_value ) {
        const std::string value = text.toStdString();
        if( m_current_value != value ) {
			try {
				m_policylib->updateElementFromString( m_key, value );
				m_current_value = value;
			} catch (...) {
				if(value == "") {
					m_current_value = "";
				}
				emit textChangeFromPolicyLib( m_current_value.c_str() );
			}
        }
    }
    else {
        // we do not support editing of key annotations
        emit textChangeFromPolicyLib( m_current_value.c_str() );
    }
}


} // of namespace qtobserver
