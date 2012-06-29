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

#include "tinia/qtobserver/moc/RadioButtonGroup.hpp"
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace tinia {
namespace qtobserver {
namespace impl {

RadioButtonGroup::RadioButtonGroup(std::string key,
                           std::shared_ptr<model::ExposedModel> model,
                                   bool horizontal,
                           QWidget *parent) :
   QGroupBox(parent), m_model(model), m_key(key)
{
   if(horizontal)
   {
      setLayout(new QHBoxLayout());
   }
   else
   {
      setLayout(new QVBoxLayout());
   }
   m_model->addStateSchemaListener(m_key, this);

   // Populate the buttons
   auto restrictions = m_model->getRestrictionSet(m_key);
   for(auto it= restrictions.begin(); it != restrictions.end(); it++)
   {
      layout()->addWidget(new RadioButton(*it, m_key, m_model, this));
   }

   setFlat(true);
}

RadioButtonGroup::~RadioButtonGroup()
{
   m_model->removeStateSchemaListener(m_key, this);
}

void RadioButtonGroup::stateSchemaElementAdded(model::StateSchemaElement *stateSchemaElement)
{
}

void RadioButtonGroup::stateSchemaElementRemoved(model::StateSchemaElement *stateSchemaElement)
{
}

void RadioButtonGroup::stateSchemaElementModified(model::StateSchemaElement *stateSchemaElement)
{
   // Do something here eventually
}
}
}
} // of namespace tinia
