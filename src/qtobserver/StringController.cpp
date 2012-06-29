/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tinia/qtobserver/moc/StringController.hpp"
#include <QLineEdit>
#include <QLabel>
#include <QAbstractButton>

namespace tinia {
namespace qtobserver {
namespace impl {

StringController::StringController( QWidget *widget,
                                    std::shared_ptr<model::ExposedModel>   model,
                                    const std::string&                      key,
                                    const bool                              show_value,
                                    const QString&                          suffix )
    : QObject( widget ),
      m_model( model ),
      m_key( key ),
      m_show_value( show_value ),
      m_suffix( suffix ),
      m_button( NULL )
{
    if( m_show_value ) {
        m_current_value = m_model->getElementValueAsString( m_key );
    }
    else {
        model::StateSchemaElement element = m_model->getStateSchemaElement(key);
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
        connect( this, SIGNAL( textChangeFromExposedModel(QString)),
                line_edit, SLOT(setText(QString)) );
    }
    QLabel* label = dynamic_cast<QLabel*>( widget );
    if( label != NULL ) {
        label->setText(  QString( m_current_value.c_str() + m_suffix ) );
        connect( this, SIGNAL( textChangeFromExposedModel(QString)),
                 label, SLOT( setText(QString)) );
    }
    QAbstractButton* button = dynamic_cast<QAbstractButton*>( widget );
    if( button != NULL ) {
        m_button = button;
        m_button->setText( QString( m_current_value.c_str() + m_suffix ) );
    }

    if( m_show_value ) {
        m_model->addStateListener( m_key, this );
    }

}

StringController::~StringController()
{
    if( m_show_value ) {
        m_model->removeStateListener( m_key, this );
    }
}

void
StringController::stateElementModified( model::StateElement *state_element )
{
    const std::string& value = state_element->getStringValue();
    if( value != m_current_value ) {
        m_current_value = value;
        if( m_button != NULL ) {
            // button doesn't have an appropriate slot
            m_button->setText( QString( m_current_value.c_str() + m_suffix ) );
        }
        else {
            emit textChangeFromExposedModel( QString( m_current_value.c_str() + m_suffix ) );
        }
    }
}


void
StringController::textChangeFromQt( const QString& text )
{
    if( m_suffix.size() != 0 ) {
        // sanity check. When we add sufficies, don't propagate back to
        // model.
        return;
    }
    if( m_show_value ) {
      const std::string value( text.toLocal8Bit() );
        if( m_current_value != value ) {
			try {
				m_model->updateElementFromString( m_key, value );
				m_current_value = value;
			} catch (...) {
				if(value == "") {
					m_current_value = "";
				}
				emit textChangeFromExposedModel( m_current_value.c_str() );
			}
        }
    }
    else {
        // we do not support editing of key annotations
        emit textChangeFromExposedModel( m_current_value.c_str() );
    }
}

}
} // of namespace qtobserver
} // of namespace tinia
