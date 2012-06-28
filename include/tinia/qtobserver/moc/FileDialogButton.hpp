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

#ifndef FILEDIALOGBUTTON_HPP
#define FILEDIALOGBUTTON_HPP

#include <QPushButton>
#include <tinia/policy/Policy.hpp>
#include <memory>
#include "tinia/qtobserver/moc/StringController.hpp"

namespace tinia {
namespace qtobserver {
class FileDialogButton : public QPushButton
{
    Q_OBJECT
public:
    explicit FileDialogButton(std::string key, bool showValue,
                              std::shared_ptr<policy::Policy> policy,
                              QWidget *parent = 0);

signals:

public slots:
   void readFile();
private:
   std::string m_key;
   std::shared_ptr<policy::Policy> m_policy;
   StringController m_controller;
};

}
}
#endif // FILEDIALOGBUTTON_HPP
