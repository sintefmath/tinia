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

#include <tinia/qtcontroller/scripting/ScriptMouseEvent.hpp>

namespace tinia {
namespace qtcontroller {
namespace scripting {

ScriptMouseEvent::ScriptMouseEvent(const QMouseEvent& event, QObject *parent) :
    QObject(parent), m_event(event)
{
    ;
}

int ScriptMouseEvent::x()
{
    return m_event.x();
}

int ScriptMouseEvent::y()
{
    return m_event.y();
}

bool ScriptMouseEvent::altKey()
{
    return m_event.modifiers() & Qt::AltModifier;
}

bool ScriptMouseEvent::shiftKey()
{
    return m_event.modifiers() & Qt::ShiftModifier;
}

bool ScriptMouseEvent::ctrlKey()
{
    return m_event.modifiers() & Qt::CTRL;
}

bool ScriptMouseEvent::metaKey()
{
    return m_event.modifiers() & Qt::MetaModifier;
}

} // namespace scripting
} // namespace qtcontroller
} // namespace tinia
