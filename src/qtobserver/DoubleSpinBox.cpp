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

#include "tinia/qtobserver/moc/DoubleSpinBox.hpp"

namespace tinia {
namespace qtobserver {
namespace impl {

DoubleSpinBox::DoubleSpinBox(std::string key, std::shared_ptr<model::ExposedModel> model,
                 QWidget *parent) :
   QDoubleSpinBox(parent), m_key(key.c_str()), m_model(model)
{
   // Get max and min
   model::StateSchemaElement element = m_model->getStateSchemaElement(m_key);
   try
   {
      double max = boost::lexical_cast<double>(element.getMaxConstraint());
      double min = boost::lexical_cast<double>(element.getMinConstraint());
      setMaximum(max);
      setMinimum(min);
   }
   catch(boost::bad_lexical_cast* exception)
   {
      // Do nothing
   }

   m_model->addStateListener(m_key, this);
   m_model->addStateSchemaListener(m_key, this);
   double value;
   m_model->getElementValue<double>(m_key, value);
   setValue(value);
   connect(this, SIGNAL(setValueFromExposedModel(double)), this,
           SLOT(setValue(double)));
   connect(this, SIGNAL(valueChanged(double)), this, SLOT(valueSetFromQt(double)));
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}

DoubleSpinBox::~DoubleSpinBox()
{
   m_model->removeStateListener(m_key, this);
   m_model->removeStateSchemaListener(m_key, this);
}


void DoubleSpinBox::stateElementModified(model::StateElement *stateElement)
{
   double value;
   m_model->getElementValue<double>(m_key, value);
   emit setValueFromExposedModel(value);
}

void DoubleSpinBox::stateSchemaElementModified(model::StateSchemaElement *stateSchemaElement)
{
    std::cout << "Updating min/max" << std::endl;
    double max, min;
    stateSchemaElement->getMinConstraint(min);
    stateSchemaElement->getMaxConstraint(max);
    setMaximum(max);
    setMinimum(min);

}

void DoubleSpinBox::valueSetFromQt(double val)
{
   m_model->updateElement(m_key, val);
}

}
}
}

