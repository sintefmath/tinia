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

#include <tinia/qtcontroller/scripting/Viewer.hpp>

namespace tinia {
namespace qtcontroller {
namespace scripting {
namespace {

void setMatrix(boost::array<float, 16>& arr, QScriptValue val) {
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
    if(key=="modelview") {
        setMatrix(m_viewer.modelviewMatrix, value);
    }
    else if(key =="projection") {
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
    if(key=="modelview") {
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
    else if(key =="projection") {
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
