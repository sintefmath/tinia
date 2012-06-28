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
SpinBox::SpinBox(std::string key, std::shared_ptr<policy::Policy> policy,
                 QWidget *parent) :
   QSpinBox(parent), m_key(key.c_str()), m_policy(policy)
{
   // Get max and min
   policy::StateSchemaElement element = m_policy->getStateSchemaElement(m_key);
   try {
	int maxValue = boost::lexical_cast<int>(element.getMaxConstraint());
	int minValue = boost::lexical_cast<int>(element.getMinConstraint());
	setMaximum(maxValue);
	setMinimum(minValue);
   } catch(...) {
	   std::cerr<< "WARNING: Using SpinBox without max/min"<<std::endl;

   }


   m_policy->addStateListener(m_key, this);
   m_policy->addStateSchemaListener(m_key, this);
   int value;
   m_policy->getElementValue<int>(m_key, value);
   setValue(value);
   connect(this, SIGNAL(setValueFromPolicy(int)), this,
           SLOT(setValue(int)));
   connect(this, SIGNAL(valueChanged(int)), this, SLOT(valueSetFromQt(int)));
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}
SpinBox::~SpinBox()
{
    m_policy->removeStateListener(m_key, this);
     m_policy->removeStateSchemaListener(m_key, this);
}

void SpinBox::stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement)
{
    int max, min;
    stateSchemaElement->getMinConstraint(min);
    stateSchemaElement->getMaxConstraint(max);
    setMaximum(max);
    setMinimum(min);
}
}

void qtobserver::SpinBox::stateElementModified(policy::StateElement *stateElement)
{
   int value;
   m_policy->getElementValue<int>(m_key, value);
   emit setValueFromPolicy(value);
}

void qtobserver::SpinBox::valueSetFromQt(int val)
{
   m_policy->updateElement(m_key, val);
}

} // of namespace tinia
