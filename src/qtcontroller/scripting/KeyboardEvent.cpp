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

#include "tinia/qtcontroller/scripting/KeyboardEvent.hpp"

namespace tinia {
namespace qtcontroller {
namespace scripting {

KeyboardEvent::KeyboardEvent(const QKeyEvent& e, QObject *parent) :
    QObject(parent), m_event(e)
{
}

int KeyboardEvent::key()
{
    return m_event.key();
}

int KeyboardEvent::keyCode()
{
    return m_event.key();
}


} // namespace scripting
} // namespace qtcontroller
} // namespace tinia
