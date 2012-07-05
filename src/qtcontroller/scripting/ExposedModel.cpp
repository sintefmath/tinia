#include <tinia/qtcontroller/scripting/ExposedModel.hpp>

namespace tinia {
namespace qtcontroller {
namespace scripting {


ExposedModel::ExposedModel(std::shared_ptr<tinia::model::ExposedModel> model, QObject *parent) :
    QObject(parent), m_model(model)
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
    m_model->updateElement(key.toStdString(), value);
}

void ExposedModel::updateElement(const QString &key, bool value)
{
    m_model->updateElement(key.toStdString(), value);
}

} // namespace scripting
} // namespace qtcontroller
} // namespace tinia
