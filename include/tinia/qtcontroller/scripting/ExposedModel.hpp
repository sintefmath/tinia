#pragma once
#include <QObject>
#include <tinia/model/ExposedModel.hpp>
#include <tinia/qtcontroller/scripting/Viewer.hpp>
#include <QtScript>
namespace tinia {
namespace qtcontroller {
namespace scripting {

class ExposedModel : public QObject
{
    Q_OBJECT
public:
    explicit ExposedModel(std::shared_ptr<tinia::model::ExposedModel> model,
                          QScriptEngine* engine, QObject *parent = 0);
    
signals:
    
public slots:
    void updateElement(const QString& key, QScriptValue value);

    QScriptValue getElementValue(const QString& key);


private:
    QScriptEngine* m_engine;
    std::shared_ptr<tinia::model::ExposedModel> m_model;
};

} // namespace scripting
} // namespace qtcontroller
} // namespace tinia


