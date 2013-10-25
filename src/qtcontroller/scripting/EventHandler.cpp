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

#include <tinia/qtcontroller/impl/EventHandler.hpp>
#include <tinia/qtcontroller/impl/ScriptEngine.hpp>
#include <tinia/qtcontroller/scripting/ScriptMouseEvent.hpp>
#include <tinia/qtcontroller/scripting/KeyboardEvent.hpp>

namespace tinia {
namespace qtcontroller {
namespace scripting {

EventHandler::EventHandler(const std::string& scriptClassName,
                           const std::string& key,
                           boost::shared_ptr<tinia::model::ExposedModel> model,
                           QScriptEngine& engine)
    : m_engine(engine),
      m_scriptModel(model, &m_engine),
      m_model(model)
{

    QScriptValue parameters = m_engine.newObject();
    parameters.setProperty("exposedModel", m_engine.newQObject(&m_scriptModel));
    parameters.setProperty("key", QString(key.c_str()));
    m_scriptHandler =
            m_engine.evaluate(QString(scriptClassName.c_str())).construct(QScriptValueList() << parameters);

    if(m_scriptHandler.isError()) {
        throw std::runtime_error("Error while creating JavaScript object " + scriptClassName + ": " + m_scriptHandler.toString().toStdString());
    }

}

EventHandler::EventHandler(const std::string &scriptClassName,
                           const std::map<std::string, std::string> parameters,
                           boost::shared_ptr<model::ExposedModel> model,
                           QScriptEngine &engine)
    : m_engine(engine),
      m_scriptModel(model, &m_engine),
      m_model(model)
{
    QScriptValue scriptParameters = m_engine.newObject();
    scriptParameters.setProperty("exposedModel", m_engine.newQObject(&m_scriptModel));

    for(std::map<std::string, std::string>::const_iterator param = parameters.begin(); param != parameters.end(); ++param) {
        scriptParameters.setProperty(QString(param->first.c_str()), QString(param->second.c_str()));
    }
    m_scriptHandler =
            m_engine.evaluate(QString(scriptClassName.c_str())).construct(QScriptValueList() << scriptParameters);

    if(m_scriptHandler.isError()) {
        throw std::runtime_error("Error while creating JavaScript object " + scriptClassName + ": " + m_scriptHandler.toString().toStdString());
    }
}


void EventHandler::mouseMoveEvent(QMouseEvent *event)
{
    ScriptMouseEvent scriptEvent(*event);
    QScriptValue error = m_scriptHandler.property("mouseMoveEvent").call(m_scriptHandler,
                                                    QScriptValueList() << m_engine.newQObject(&scriptEvent));

    if(error.isError()) {
        throw std::runtime_error("Error in Script: " + error.toString().toStdString());
    }
}

void EventHandler::mousePressEvent(QMouseEvent *event)
{
    ScriptMouseEvent scriptEvent(*event);
    QScriptValue error = m_scriptHandler.property("mousePressEvent").call(m_scriptHandler,
                                                    QScriptValueList() << m_engine.newQObject(&scriptEvent));

    if(error.isError()) {
        throw std::runtime_error("Error in Script: " + error.toString().toStdString());
    }
}

void EventHandler::mouseReleaseEvent(QMouseEvent *event)
{
    ScriptMouseEvent scriptEvent(*event);
    QScriptValue error = m_scriptHandler.property("mouseReleaseEvent").call(m_scriptHandler,
                                                       QScriptValueList() << m_engine.newQObject(&scriptEvent));

    if(error.isError()) {
        throw std::runtime_error("Error in Script: " + error.toString().toStdString());
    }
}

void EventHandler::keyPressEvent(QKeyEvent *event)
{
    KeyboardEvent scriptEvent(*event);
    QScriptValue error = m_scriptHandler.property("keyPressEvent").call(m_scriptHandler,
                                                       QScriptValueList() << m_engine.newQObject(&scriptEvent));

    if(error.isError()) {
        throw std::runtime_error("Error in Script: " + error.toString().toStdString());
    }
}

} // namespace scripting
} // namespace qtcontroller
} // namespace tinia
