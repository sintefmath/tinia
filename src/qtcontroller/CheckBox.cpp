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

#include "tinia/qtcontroller/moc/CheckBox.hpp"
#include "tinia/qtcontroller/impl/utils.hpp"

namespace tinia {
namespace qtcontroller {
namespace impl {

CheckBox::CheckBox(std::string key, boost::shared_ptr<model::ExposedModel> model,
                   QWidget *parent) :
   QCheckBox(parent), m_key(key), m_model(model)
{
   setText(prettyName(m_key, m_model).c_str());
   m_model->addStateListener(m_key, this);
   bool checked;
   m_model->getElementValue<bool>(m_key, checked);
   setChecked(checked);

   connect(this, SIGNAL(setCheckFromExposedModel(bool)), this,
           SLOT(setChecked(bool)));
   connect(this, SIGNAL(toggled(bool)), this,
           SLOT(setCheckedFromQt(bool)));
}

CheckBox::~CheckBox()
{
   m_model->removeStateListener(m_key, this);
}


void CheckBox::setCheckedFromQt(bool checked)
{
   m_model->updateElement<bool>(m_key, checked);
}

void CheckBox::stateElementModified(model::StateElement *stateElement)
{
   bool checked;
   stateElement->getValue(checked);
   emit setCheckFromExposedModel(checked);
}

}
}
}
