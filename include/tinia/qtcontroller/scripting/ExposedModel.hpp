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


