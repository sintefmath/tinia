#include <tinia/qtcontroller/scripting/Viewer.hpp>

namespace tinia {
namespace qtcontroller {
namespace scripting {
namespace {

void setMatrix(std::array<float, 16>& arr, QScriptValue val) {
    for(int i = 0; i < 16; ++i) {
        arr[i] = val.property(i).toNumber();
    }
}

}
Viewer::Viewer(QScriptEngine* engine, QObject *parent) :
    QObject(parent), m_engine(engine)
{
}

model::Viewer &Viewer::viewer()
{
    return m_viewer;
}

void Viewer::updateElement(const QString& key, QScriptValue value)
{
    if(key=="modelviewMatrix") {
        setMatrix(m_viewer.modelviewMatrix, value);
    }
    else if(key =="projectionMatrix") {
        setMatrix(m_viewer.projectionMatrix, value);
    }
    else if(key=="height") {
        m_viewer.height = int(value.toNumber());
    }
    else if(key=="width") {
        m_viewer.width = int(value.toNumber());
    }
}

QScriptValue Viewer::getElementValue(const QString &key)
{
    if(key=="modelviewMatrix") {
        QString array("Array(");
        for(int i = 0; i < 16; ++i) {
            array +=QString::number(m_viewer.modelviewMatrix[i]);
            if( i < 15 ) {
                array += ", ";
            }
        }
        array +=")";
        return m_engine->evaluate(array);

    }
    else if(key =="projectionMatrix") {
        QString array("Array(");
        for(int i = 0; i < 16; ++i) {
            array +=QString::number(m_viewer.projectionMatrix[i]);
            if( i < 15 ) {
                array += ", ";
            }
        }
        array +=")";
        return m_engine->evaluate(array);
    }
    else if(key=="height") {
        return QScriptValue(m_viewer.height);
    }
    else if(key=="width") {
        return QScriptValue(m_viewer.width);
    }
    return QScriptValue();
}


} // namespace scripting
} // namespace qtobserver
} // namespace tinia
