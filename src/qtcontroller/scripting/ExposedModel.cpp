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

#include <tinia/qtcontroller/scripting/ExposedModel.hpp>

namespace tinia {
namespace qtcontroller {
namespace scripting {

ExposedModel::ExposedModel(std::shared_ptr<tinia::model::ExposedModel> model,
                           QScriptEngine *engine,
                           QObject *parent)
    : QObject(parent), m_engine(engine), m_model(model)
{
    m_model->addStateListener(this);
}

ExposedModel::~ExposedModel()
{
    m_model->removeStateListener(this);
}

void ExposedModel::stateElementModified(model::StateElement *stateElement)
{
    auto listenersFound = m_listeners.find(stateElement->getKey());
    if(listenersFound != m_listeners.end()) {
        auto& listeners = listenersFound->second;
        for(size_t i = 0; i < listeners.size(); ++i) {
            listeners[i].call(QScriptValue(), QScriptValueList()
                              << QString(stateElement->getKey().c_str())
                              << getElementValue(QString(stateElement->getKey().c_str())));
        }
    }
}

void ExposedModel::updateElement(const QString &key, QScriptValue value)
{
    auto schemaElement = m_model->getStateSchemaElement(key.toStdString());
    auto type = schemaElement.getXSDType();

    if(type.find("xsd:") != std::string::npos) {
        type = type.substr(4);
    }


    if (type == std::string("double")) {
        m_model->updateElement(key.toStdString(), double(value.toNumber()));
    }
    else if(type==std::string("integer"))  {
        m_model->updateElement(key.toStdString(), int(value.toNumber()));
    }
    else if(type==std::string("bool")) {
        m_model->updateElement(key.toStdString(), value.toBool());
    }
    else if(type==std::string("string")) {
        m_model->updateElement(key.toStdString(), value.toString().toStdString());
    }
    else if(type==std::string("complexType")) {
        m_model->updateElement(key.toStdString(), static_cast<Viewer*>(value.toQObject())->viewer());
    }

}


QScriptValue ExposedModel::getElementValue(const QString &key)
{
    auto schemaElement = m_model->getStateSchemaElement(key.toStdString());
    auto type = schemaElement.getXSDType();
    if(type.find("xsd:") != std::string::npos) {
        type = type.substr(4);
    }

    if (type == std::string("double")) {
        double value;
        m_model->getElementValue(key.toStdString(), value);
        return QScriptValue(value);
    }
    if (type == std::string("integer")) {
        int value;
        m_model->getElementValue(key.toStdString(), value);
        return QScriptValue(value);
    }
    if (type == std::string("bool")) {
        bool value;
        m_model->getElementValue(key.toStdString(), value);
        return QScriptValue(value);
    }
    if (type == std::string("string")) {
        std::string value;
        m_model->getElementValue(key.toStdString(), value);
        return QScriptValue(QString(value.c_str()));
    }
    if (type == std::string("complexType")) {
        auto v = new Viewer(m_engine, this);
        m_model->getElementValue(key.toStdString(), v->viewer());
        return m_engine->newQObject(v);
    }
    return QScriptValue();
}

void ExposedModel::addLocalListener(const QString &key, QScriptValue function)
{
    m_listeners[key.toStdString()].push_back(function);
}

} // namespace scripting
} // namespace qtcontroller
} // namespace tinia
