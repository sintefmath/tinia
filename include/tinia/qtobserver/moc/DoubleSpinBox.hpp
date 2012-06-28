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

#ifndef QTOBSERVER_DOUBLESPINBOX_HPP
#define QTOBSERVER_DOUBLESPINBOX_HPP

#include <QDoubleSpinBox>
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/StateListener.hpp"
#include "tinia/policy/StateSchemaListener.hpp"
#include <memory>

namespace tinia {
namespace qtobserver {
/**
  \todo Use max and min if available.
  */
class DoubleSpinBox : public QDoubleSpinBox, public policy::StateListener, public policy::StateSchemaListener
{
    Q_OBJECT
public:
    explicit DoubleSpinBox(std::string key, std::shared_ptr<policy::Policy> policy,
                     QWidget *parent = 0);
   ~DoubleSpinBox();

   void stateElementModified(policy::StateElement *stateElement);
   void stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement);
   void stateSchemaElementAdded(policy::StateSchemaElement *stateSchemaElement) {}
   void stateSchemaElementRemoved(policy::StateSchemaElement *stateSchemaElement) {}
signals:
   void setValueFromPolicy(double val);
public slots:
   void valueSetFromQt(double val);

private:
   std::string m_key;
   std::shared_ptr<policy::Policy> m_policy;
};
}
}
#endif // QTOBSERVER_DOUBLESPINBOX_HPP
