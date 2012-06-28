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

#ifndef QTOBSERVER_SPINBOX_HPP
#define QTOBSERVER_SPINBOX_HPP

#include <QSpinBox>
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/StateListener.hpp"
#include <tinia/policy/StateSchemaListener.hpp>
#include <memory>

namespace tinia {
namespace qtobserver {
/**
  \todo Use max and min if available.
  */
class SpinBox : public QSpinBox, public policy::StateListener, public policy::StateSchemaListener
{
    Q_OBJECT
public:
    explicit SpinBox(std::string key, std::shared_ptr<policy::Policy> policy,
                     QWidget *parent = 0);
    ~SpinBox();
    void stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement);
    void stateSchemaElementAdded(policy::StateSchemaElement *stateSchemaElement) {}
    void stateSchemaElementRemoved(policy::StateSchemaElement *stateSchemaElement) {}
    void stateElementModified(policy::StateElement *stateElement);

signals:
    void setValueFromPolicy(int val);
public slots:
    void valueSetFromQt(int val);

private:
    std::string m_key;
    std::shared_ptr<policy::Policy> m_policy;
};
}
}
#endif // QTOBSERVER_SPINBOX_HPP
