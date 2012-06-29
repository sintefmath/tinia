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
#include "tinia/qtobserver/impl/utils.hpp"
#include <QHBoxLayout>

namespace tinia {
namespace qtobserver {
namespace impl {

ElementGroup::ElementGroup(std::string key, bool showLabel,
                           std::shared_ptr<model::ExposedModel> model,
                           QWidget *parent) :
    QGroupBox(parent), m_key(key), m_model(model)
{
/*

    //   setLayout(new QHBoxLayout(this));
   m_model->addStateListener(m_key, this);
   connect(this, SIGNAL(setVisibleFromExposedModel(bool)), this,
           SLOT(setVisible(bool)));
   if(showLabel)
   {
      setTitle(prettyName(m_key, m_model).c_str());
   }

   // Should we show ourselves?
   bool show;
   m_model->getElementValue(m_key, show);
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
//   m_model->removeStateListener(m_key, this);
}


void ElementGroup::stateElementModified(model::StateElement *stateElement)
{
/*
    bool visible;
   stateElement->getValue(visible);
   emit setVisibleFromExposedModel(visible);
*/
}

}
}
}// of namespace tinia

