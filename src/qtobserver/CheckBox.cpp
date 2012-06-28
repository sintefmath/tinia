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

#include "tinia/qtobserver/moc/CheckBox.hpp"
#include "tinia/qtobserver/utils.hpp"

namespace tinia {
namespace qtobserver {

CheckBox::CheckBox(std::string key, std::shared_ptr<policy::Policy> policy,
                   QWidget *parent) :
   QCheckBox(parent), m_key(key), m_policy(policy)
{
   setText(prettyName(m_key, m_policy).c_str());
   m_policy->addStateListener(m_key, this);
   bool checked;
   m_policy->getElementValue<bool>(m_key, checked);
   setChecked(checked);

   connect(this, SIGNAL(setCheckFromPolicy(bool)), this,
           SLOT(setChecked(bool)));
   connect(this, SIGNAL(toggled(bool)), this,
           SLOT(setCheckedFromQt(bool)));
}

CheckBox::~CheckBox()
{
   m_policy->removeStateListener(m_key, this);
}
}

void qtobserver::CheckBox::setCheckedFromQt(bool checked)
{
   m_policy->updateElement<bool>(m_key, checked);
}

void qtobserver::CheckBox::stateElementModified(policy::StateElement *stateElement)
{
   bool checked;
   stateElement->getValue(checked);
   emit setCheckFromPolicy(checked);
}

} // of namespace tinia
