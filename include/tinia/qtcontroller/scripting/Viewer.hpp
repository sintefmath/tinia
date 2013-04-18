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

#pragma once
#include <QObject>
#include <tinia/model/Viewer.hpp>
#include <QtScript>
#include <QString>

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

