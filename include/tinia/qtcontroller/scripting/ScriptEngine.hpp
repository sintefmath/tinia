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
#include <memory>
#include <QScriptEngine>

namespace tinia {
namespace qtcontroller {
namespace scripting {

/** A singleton holding the script engine.
 * @note This is *not* thread safe.
 */
class ScriptEngine
{
public:
    static std::shared_ptr<ScriptEngine> getInstance();

    QScriptEngine& engine();
    const QScriptEngine& engine() const;

private:
    QScriptEngine m_engine;
    static std::shared_ptr<ScriptEngine> m_instance;

    ScriptEngine();
    ScriptEngine(const ScriptEngine&);
    ScriptEngine& operator=(const ScriptEngine&);
};

} // namespace scripting
} // namespace qtcontroller
} // namespace tinia

