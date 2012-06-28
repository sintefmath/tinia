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

#ifndef QTOBSERVER_TABWIDGETCHILDREN_HPP
#define QTOBSERVER_TABWIDGETCHILDREN_HPP

#include <QWidget>
#include <QTableWidget>
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/StateSchemaListener.hpp"

namespace tinia {
namespace qtobserver {

class TabWidgetChildren : public QWidget, public policy::StateSchemaListener
{
    Q_OBJECT
public:
    explicit TabWidgetChildren(std::string key,
                               std::shared_ptr<policy::Policy> policy,
                               QTabWidget *parent = 0);
   void stateSchemaElementAdded(policy::StateSchemaElement *stateSchemaElement) {}
   void stateSchemaElementRemoved(policy::StateSchemaElement *stateSchemaElement) {}
   void stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement);

signals:

public slots:
private:
      std::shared_ptr<policy::Policy> m_policy;
      std::string m_key;

};

} // namespace qtobserver
} // of namespace tinia
#endif // QTOBSERVER_TABWIDGETCHILDREN_HPP
