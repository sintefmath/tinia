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
#include <QtScript>
#include <QMouseEvent>

namespace tinia {
namespace qtcontroller {
namespace scripting {

/** MouseEvent meant to resemble the JavaScript mouseevent as much as possible.
 */
class ScriptMouseEvent : public QObject
{
    Q_OBJECT
public:
    explicit ScriptMouseEvent(const QMouseEvent& e, QObject *parent = 0);

    Q_PROPERTY(int relativeX READ relativeX)
    Q_PROPERTY(int relativeY READ relativeY)
    Q_PROPERTY(bool altKey READ altKey)
    Q_PROPERTY(bool shiftKey READ shiftKey)
    Q_PROPERTY(bool ctrlKey READ ctrlKey)
    Q_PROPERTY(bool metaKey READ metaKey)
    Q_PROPERTY(int button READ button)
    
signals:
    
public slots:
    int button();
    int relativeX();
    int relativeY();
    bool altKey();
    bool shiftKey();
    bool ctrlKey();
    bool metaKey();

private:
    QMouseEvent m_event;
};

} // namespace scripting
} // namespace qtcontroller
} // namespace tinia

