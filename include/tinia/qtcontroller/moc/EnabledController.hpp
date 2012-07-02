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

#pragma once

#include <QObject>
#include <QWidget>
#include <string>
#include "tinia/model/ExposedModel.hpp"
#include "tinia/model/StateListener.hpp"

namespace tinia {
namespace qtcontroller {
namespace impl {

class EnabledController : public QObject, public model::StateListener
{
    Q_OBJECT;
public:
    explicit
    EnabledController( QWidget*                               widget,
                       std::shared_ptr<model::ExposedModel>  model,
                       const std::string&                     key,
                       const bool                             inverted );

    ~EnabledController();

    void
    stateElementModified(model::StateElement *stateElement);

signals:
    void
    setWidgetEnabled( bool enabled );

protected:
    std::shared_ptr<model::ExposedModel>   m_model;
    const std::string                       m_key;
    const bool                              m_inverted;
};


}
} // of namespace qtcontroller
} // of namespace tinia
