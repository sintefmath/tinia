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

#include <QWidget>
#include <QTableWidget>
#include "tinia/model/ExposedModel.hpp"
#include "tinia/model/StateSchemaListener.hpp"

namespace tinia {
namespace qtcontroller {
namespace impl {

class TabWidgetChildren : public QWidget, public model::StateSchemaListener
{
    Q_OBJECT
public:
    explicit TabWidgetChildren(std::string key,
                               boost::shared_ptr<model::ExposedModel> model,
                               QTabWidget *parent = 0);
   void stateSchemaElementAdded(model::StateSchemaElement *stateSchemaElement) {}
   void stateSchemaElementRemoved(model::StateSchemaElement *stateSchemaElement) {}
   void stateSchemaElementModified(model::StateSchemaElement *stateSchemaElement);

signals:

public slots:
private:
      boost::shared_ptr<model::ExposedModel> m_model;
      std::string m_key;

};

}
} // namespace qtcontroller
} // of namespace tinia
