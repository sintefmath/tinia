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
DoubleSpinBox::DoubleSpinBox(std::string key, std::shared_ptr<policy::Policy> policy,
                 QWidget *parent) :
   QDoubleSpinBox(parent), m_key(key.c_str()), m_policy(policy)
{
   // Get max and min
   policy::StateSchemaElement element = m_policy->getStateSchemaElement(m_key);
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

   m_policy->addStateListener(m_key, this);
   m_policy->addStateSchemaListener(m_key, this);
   double value;
   m_policy->getElementValue<double>(m_key, value);
   setValue(value);
   connect(this, SIGNAL(setValueFromPolicy(double)), this,
           SLOT(setValue(double)));
   connect(this, SIGNAL(valueChanged(double)), this, SLOT(valueSetFromQt(double)));
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}
DoubleSpinBox::~DoubleSpinBox()
{
   m_policy->removeStateListener(m_key, this);
   m_policy->removeStateSchemaListener(m_key, this);
}
}

void qtobserver::DoubleSpinBox::stateElementModified(policy::StateElement *stateElement)
{
   double value;
   m_policy->getElementValue<double>(m_key, value);
   emit setValueFromPolicy(value);
}

void qtobserver::DoubleSpinBox::stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement)
{
    std::cout << "Updating min/max" << std::endl;
    double max, min;
    stateSchemaElement->getMinConstraint(min);
    stateSchemaElement->getMaxConstraint(max);
    setMaximum(max);
    setMinimum(min);

}

void qtobserver::DoubleSpinBox::valueSetFromQt(double val)
{
   m_policy->updateElement(m_key, val);
}
}

