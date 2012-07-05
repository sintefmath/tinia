#include <tinia/qtcontroller/scripting/ExposedModel.hpp>

namespace tinia {
namespace qtcontroller {
namespace scripting {


ExposedModel::ExposedModel(std::shared_ptr<tinia::model::ExposedModel> model, QObject *parent) :
    QObject(parent)
{
}

void ExposedModel::updateElement(const QString &key, const QString &value)
{
    auto schemaElement = m_model->getStateSchemaElement(key.toStdString());
    auto type = schemaElement.getXSDType();

    if(type.find("xsd:") != std::string::npos) {
        type = type.substr(4);
    }


    if (type == "double") {
        updateElement(key, boost::lexical_cast<double>(value.toStdString()));
    }
    else if(type=="int")  {
        updateElement(key, boost::lexical_cast<int>(value.toStdString()));
    }
    else if(type=="bool") {
        updateElement(key, boost::lexical_cast<bool>(value.toStdString()));
    }
    else if(type=="string") {
        m_model->updateElement(key.toStdString(), value.toStdString());
    }

}

void ExposedModel::updateElement(const QString &key, int value)
{
}

void ExposedModel::updateElement(const QString &key, double value)
{
}

void ExposedModel::updateElement(const QString &key, bool value)
{
}

} // namespace scripting
} // namespace qtcontroller
} // namespace tinia
