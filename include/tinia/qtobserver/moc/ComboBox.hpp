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

#ifndef QTOBSERVER_COMBOBOX_HPP
#define QTOBSERVER_COMBOBOX_HPP

#include <QComboBox>
#include <QStringList>
#include <memory>
#include <tinia/policy/Policy.hpp>
#include <tinia/policy/StateListener.hpp>
#include <tinia/policy/StateSchemaListener.hpp>

namespace tinia {
namespace qtobserver {

class ComboBox : public QComboBox, public policy::StateListener,
         public policy::StateSchemaListener
{
    Q_OBJECT
public:
    explicit ComboBox(std::string key, std::shared_ptr<policy::Policy> policy,
                      QWidget *parent = 0);
   ~ComboBox();

   void stateElementModified(policy::StateElement *stateElement);
   void stateSchemaElementAdded(policy::StateSchemaElement *stateSchemaElement);
   void stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement);
   void stateSchemaElementRemoved(policy::StateSchemaElement *stateSchemaElement);

signals:
   void setStateFromPolicy(int);
   void clearFromPolicy();
   void addItemsFromPolicy(const QStringList& list);

public slots:

private slots:
   void activatedChanged(QString value);
private:
   QStringList m_options;
   std::shared_ptr<policy::Policy> m_policy;
   std::string m_key;
};

} // namespace qtobserver
} // namespace tinia

#endif // QTOBSERVER_COMBOBOX_HPP
