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

#include <tinia/qtcontroller/scripting/EventHandler.hpp>
#include <tinia/qtcontroller/scripting/ScriptEngine.hpp>

namespace tinia {
namespace qtcontroller {
namespace scripting {

EventHandler::EventHandler(const std::string& scriptClassName,
                           std::shared_ptr<tinia::model::ExposedModel> model)
    : m_scriptModel(model, &ScriptEngine::getInstance()->engine()),
      m_model(model)
{
    auto& engine = ScriptEngine::getInstance()->engine();
    m_scriptHandler =
            engine.evaluate(QString(scriptClassName.c_str())).construct(QScriptValueList() << engine.newQObject(&m_scriptModel));

}

} // namespace scripting
} // namespace qtcontroller
} // namespace tinia
