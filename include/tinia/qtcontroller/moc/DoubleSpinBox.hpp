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

#include <QDoubleSpinBox>
#include "tinia/model/ExposedModel.hpp"
#include "tinia/model/StateListener.hpp"
#include "tinia/model/StateSchemaListener.hpp"
#include <memory>

namespace tinia {
namespace qtcontroller {
namespace impl {
/**
  \todo Use max and min if available.
  */
class DoubleSpinBox : public QDoubleSpinBox, public model::StateListener, public model::StateSchemaListener
{
    Q_OBJECT
public:
    explicit DoubleSpinBox(std::string key, boost::shared_ptr<model::ExposedModel> model,
                     QWidget *parent = 0);
   ~DoubleSpinBox();

   void stateElementModified(model::StateElement *stateElement);
   void stateSchemaElementModified(model::StateSchemaElement *stateSchemaElement);
   void stateSchemaElementAdded(model::StateSchemaElement *stateSchemaElement) {}
   void stateSchemaElementRemoved(model::StateSchemaElement *stateSchemaElement) {}
signals:
   void setValueFromExposedModel(double val);
public slots:
   void valueSetFromQt(double val);

private:
   std::string m_key;
   boost::shared_ptr<model::ExposedModel> m_model;
};
}
}
}
