#include <tinia/qtcontroller/scripting/ExposedModel.hpp>

namespace tinia {
namespace qtcontroller {
namespace scripting {


ExposedModel::ExposedModel(std::shared_ptr<tinia::model::ExposedModel> model,
                           QScriptEngine *engine,
                           QObject *parent)
    : QObject(parent), m_engine(engine), m_model(model)
{
}

void ExposedModel::updateElement(const QString &key, const QString &value)
{
    auto schemaElement = m_model->getStateSchemaElement(key.toStdString());
    auto type = schemaElement.getXSDType();

    if(type.find("xsd:") != std::string::npos) {
        type = type.substr(4);
    }


    if (type == std::string("double")) {
        updateElement(key, boost::lexical_cast<double>(value.toStdString()));
    }
    else if(type==std::string("integer"))  {
        updateElement(key, boost::lexical_cast<int>(value.toStdString()));
    }
    else if(type==std::string("bool")) {
        updateElement(key, boost::lexical_cast<bool>(value.toStdString()));
    }
    else if(type==std::string("string")) {
        m_model->updateElement(key.toStdString(), value.toStdString());
    }

}

void ExposedModel::updateElement(const QString &key, int value)
{
    m_model->updateElement(key.toStdString(), value);
}

void ExposedModel::updateElement(const QString &key, double value)
{
    // Here we actually need to check the type, since it could be an int,
    // as QtScript treats them all as doubles.
    if(m_model->getStateSchemaElement(key.toStdString()).getXSDType()=="xsd:integer") {
        updateElement(key, int(value));
    }
    else {
        m_model->updateElement(key.toStdString(), value);
    }
}

void ExposedModel::updateElement(const QString &key, bool value)
{
    m_model->updateElement(key.toStdString(), value);
}

void ExposedModel::updateElement(const QString &key, Viewer *v)
{
    m_model->updateElement(key.toStdString(), v->viewer());
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

} // namespace scripting
} // namespace qtcontroller
} // namespace tinia
