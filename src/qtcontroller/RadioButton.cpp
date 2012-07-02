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

#include "tinia/qtcontroller/moc/RadioButton.hpp"

namespace tinia {
namespace qtcontroller {
namespace impl {

RadioButton::RadioButton(std::string value, std::string key,
                         std::shared_ptr<model::ExposedModel> model,
                         QWidget *parent) :
   QRadioButton(value.c_str(), parent), m_value(value), m_key(key), m_model(model)
{
   // Connect signals
   connect(this, SIGNAL(toggled(bool)), this, SLOT(setCheckedFromQt(bool)));
   connect(this, SIGNAL(setCheckedFromExposedModel(bool)), this, SLOT(setChecked(bool)));

   m_model->addStateListener(m_key, this);
   if(m_model->getElementValueAsString(m_key) == m_value)
   {
      setChecked(true);
   }

}
RadioButton::~RadioButton()
{
   m_model->removeStateListener(m_key, this);
}


void RadioButton::setCheckedFromQt(bool checked)
{
   if(checked)
   {
      m_model->updateElementFromString(m_key, m_value);
   }
}

void RadioButton::stateElementModified(model::StateElement *stateElement)
{
   if(stateElement->getStringValue() == m_value)
   {
      if(isChecked())
      {
         // Don't do anything, we're already good.
      }
      else
      {
         emit setCheckedFromExposedModel(true);
      }

   }
}

}
}
} // of namespace tinia
