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
#include <QString>
#include <QAbstractButton>
#include <string>
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/StateListener.hpp"

namespace tinia {
namespace qtobserver {

class StringController
        : public QObject,
          public policy::StateListener
{
    Q_OBJECT;
public:
    explicit
    StringController( QWidget*                               widget,
                      std::shared_ptr<policy::Policy>  policy,
                      const std::string&                     key,
                      const bool                             show_value,
                      const QString&                         suffix = "" );

    ~StringController();

    void
    stateElementModified(policy::StateElement *stateElement);

signals:
    void
    textChangeFromPolicy( const QString& text );

public slots:

    void
    textChangeFromQt( const QString& text );

private:
    std::shared_ptr<policy::Policy>   m_policy;
    const std::string                       m_key;
    const bool                              m_show_value;
    std::string                             m_current_value;
    QString                                 m_suffix;
    QAbstractButton*                        m_button;
};






}
}
