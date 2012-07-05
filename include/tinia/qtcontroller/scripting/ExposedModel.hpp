#pragma once
#include <QObject>
#include <tinia/model/ExposedModel.hpp>
#include <QtScript>
namespace tinia {
namespace qtcontroller {
namespace scripting {

class ExposedModel : public QObject
{
    Q_OBJECT
public:
    explicit ExposedModel(std::shared_ptr<tinia::model::ExposedModel> model, QObject *parent = 0);
    
signals:
    
public slots:
    void updateElement(const QString& key, const QString& value);
    void updateElement(const QString& key, int value);
    void updateElement(const QString& key, double value);
    void updateElement(const QString& key, bool value);

    QScriptValue getElementValue(const QString& key);


private:
    std::shared_ptr<tinia::model::ExposedModel> m_model;
    
};

} // namespace scripting
} // namespace qtcontroller
} // namespace tinia


