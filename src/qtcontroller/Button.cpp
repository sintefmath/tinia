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

#include "tinia/qtcontroller/moc/Button.hpp"
#include "tinia/qtcontroller/impl/utils.hpp"

namespace tinia {
namespace qtcontroller {
namespace impl {

Button::Button(std::string key, boost::shared_ptr<model::ExposedModel> model,
               QWidget *parent) :
   QPushButton(parent), m_key(key), m_model(model)
{
   connect(this, SIGNAL(clicked()), this, SLOT(clickedButton()));
   setText(prettyName(key, m_model).c_str());
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}

Button::~Button()
{
}

void Button::clickedButton()
{
   m_model->updateElement<bool>(m_key, true);
}

}
}
}
