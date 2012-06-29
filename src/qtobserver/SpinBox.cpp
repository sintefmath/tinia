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

#include "tinia/qtobserver/moc/SpinBox.hpp"

namespace tinia {
namespace qtobserver {
namespace impl {


SpinBox::SpinBox(std::string key, std::shared_ptr<model::ExposedModel> model,
                 QWidget *parent) :
   QSpinBox(parent), m_key(key.c_str()), m_model(model)
{
   // Get max and min
   model::StateSchemaElement element = m_model->getStateSchemaElement(m_key);
   try {
	int maxValue = boost::lexical_cast<int>(element.getMaxConstraint());
	int minValue = boost::lexical_cast<int>(element.getMinConstraint());
	setMaximum(maxValue);
	setMinimum(minValue);
   } catch(...) {
	   std::cerr<< "WARNING: Using SpinBox without max/min"<<std::endl;

   }


   m_model->addStateListener(m_key, this);
   m_model->addStateSchemaListener(m_key, this);
   int value;
   m_model->getElementValue<int>(m_key, value);
   setValue(value);
   connect(this, SIGNAL(setValueFromExposedModel(int)), this,
           SLOT(setValue(int)));
   connect(this, SIGNAL(valueChanged(int)), this, SLOT(valueSetFromQt(int)));
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}
SpinBox::~SpinBox()
{
    m_model->removeStateListener(m_key, this);
     m_model->removeStateSchemaListener(m_key, this);
}

void SpinBox::stateSchemaElementModified(model::StateSchemaElement *stateSchemaElement)
{
    int max, min;
    stateSchemaElement->getMinConstraint(min);
    stateSchemaElement->getMaxConstraint(max);
    setMaximum(max);
    setMinimum(min);
}


void SpinBox::stateElementModified(model::StateElement *stateElement)
{
   int value;
   m_model->getElementValue<int>(m_key, value);
   emit setValueFromExposedModel(value);
}

void SpinBox::valueSetFromQt(int val)
{
   m_model->updateElement(m_key, val);
}

}
}
} // of namespace tinia
