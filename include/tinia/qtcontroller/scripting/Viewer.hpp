#pragma once
#include <QObject>
#include <tinia/model/Viewer.hpp>
#include <QtScript>

namespace tinia {
namespace qtcontroller {
namespace scripting {

class Viewer : public QObject
{
    Q_OBJECT
public:
    explicit Viewer(QScriptEngine* engine, QObject *parent = 0);
    tinia::model::Viewer& viewer();

signals:
    
public slots:
     void updateElement(const QString& key, QScriptValue value);
     QScriptValue getElementValue(const QString& key);

private:
     QScriptEngine* m_engine;
     tinia::model::Viewer m_viewer;

    
};

} // namespace scripting
} // namespace qtobserver
} // namespace tinia

