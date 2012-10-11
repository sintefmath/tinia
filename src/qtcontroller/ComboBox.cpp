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

#include "tinia/qtcontroller/moc/ComboBox.hpp"

namespace tinia {
namespace qtcontroller {
namespace impl {
ComboBox::ComboBox(std::string key, std::shared_ptr<model::ExposedModel> model,
                   QWidget *parent) :
   QComboBox(parent), m_model(model), m_key(key)
{
   // Set up signals
   connect(this, SIGNAL(setStateFromExposedModel(int)), this,
           SLOT(setCurrentIndex(int)));
   connect(this, SIGNAL(activated(QString)), this,
           SLOT(activatedChanged(QString)));

   m_model->addStateListener(m_key, this);
   m_model->addStateSchemaListener(m_key, this);



   auto restrictionSet = m_model->getRestrictionSet(m_key);
   for(auto it = restrictionSet.begin(); it != restrictionSet.end(); it++)
   {
      m_options.append(it->c_str());
   }
   m_options.sort();
   addItems(m_options);
   setCurrentIndex(m_options.indexOf(m_model->getElementValueAsString(m_key).c_str()));
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   connect(this, SIGNAL(newRestrictionsFromExposedModel()),
           this, SLOT(updateRestrictions()));

}


void ComboBox::stateElementModified(model::StateElement *stateElement)
{
   int index = m_options.indexOf(stateElement->getStringValue().c_str());
   emit setStateFromExposedModel(index);
}

void ComboBox::stateSchemaElementAdded(model::StateSchemaElement *stateSchemaElement)
{
}

void ComboBox::stateSchemaElementModified(model::StateSchemaElement *stateSchemaElement)
{
    emit newRestrictionsFromExposedModel();
}

void ComboBox::stateSchemaElementRemoved(model::StateSchemaElement *stateSchemaElement)
{
}

ComboBox::~ComboBox()
{
   m_model->removeStateSchemaListener(m_key, this);
   m_model->removeStateListener(m_key, this);

}

void ComboBox::activatedChanged(QString value)
{
    m_model->updateElementFromString(m_key, std::string( value.toLocal8Bit() ));
}

void ComboBox::updateRestrictions()
{
    // We must update the restriction set
    clear();
    m_options.clear();
    auto restrictionSet = m_model->getRestrictionSet(m_key);
    for(auto it = restrictionSet.begin(); it != restrictionSet.end(); it++)
    {
       m_options.append(it->c_str());
    }
    m_options.sort();


    addItems(m_options);
}

}
}
}
