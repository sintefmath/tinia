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

#include "tinia/qtcontroller/moc/RadioButtonGroup.hpp"
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace tinia {
namespace qtcontroller {
namespace impl {

RadioButtonGroup::RadioButtonGroup(std::string key,
                                   boost::shared_ptr<model::ExposedModel> model,
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

    addButtons();
    setFlat(true);

    connect(this, SIGNAL(newRestrictionReceived()), this, SLOT(updateRestriction()));
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
    emit newRestrictionReceived();
}

void RadioButtonGroup::updateRestriction()
{
    QLayoutItem *item;
    while((item = layout()->takeAt(0))) {
        if (item->layout()) {

            delete item->layout();
        }
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    addButtons();
}

void RadioButtonGroup::addButtons()
{

    // Populate the buttons
    std::set<std::string> restrictions = m_model->getRestrictionSet(m_key);
    for(std::set<std::string>::iterator it= restrictions.begin(); it != restrictions.end(); it++)
    {
        layout()->addWidget(new RadioButton(*it, m_key, m_model, this));
    }
}
}
}
} // of namespace tinia
