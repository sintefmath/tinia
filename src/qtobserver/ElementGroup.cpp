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

#include "tinia/qtobserver/moc/ElementGroup.hpp"
#include "tinia/qtobserver/utils.hpp"
#include <QHBoxLayout>

namespace tinia {
namespace qtobserver {

ElementGroup::ElementGroup(std::string key, bool showLabel,
                           std::shared_ptr<policy::Policy> policy,
                           QWidget *parent) :
    QGroupBox(parent), m_key(key), m_policy(policy)
{
/*

    //   setLayout(new QHBoxLayout(this));
   m_policy->addStateListener(m_key, this);
   connect(this, SIGNAL(setVisibleFromPolicy(bool)), this,
           SLOT(setVisible(bool)));
   if(showLabel)
   {
      setTitle(prettyName(m_key, m_policy).c_str());
   }

   // Should we show ourselves?
   bool show;
   m_policy->getElementValue(m_key, show);
   if(!show)
   {
      setVisible(false);
   }
*/
   // We don't want frames
   setFlat(true);

}

ElementGroup::~ElementGroup()
{
//   m_policy->removeStateListener(m_key, this);
}
}

void qtobserver::ElementGroup::stateElementModified(policy::StateElement *stateElement)
{
/*
    bool visible;
   stateElement->getValue(visible);
   emit setVisibleFromPolicy(visible);
*/
}

} // of namespace tinia

