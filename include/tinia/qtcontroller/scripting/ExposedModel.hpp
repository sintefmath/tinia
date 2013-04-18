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

class ExposedModel : public QObject, public tinia::model::StateListener
{
    Q_OBJECT
public:
    explicit ExposedModel(boost::shared_ptr<tinia::model::ExposedModel> model,
                          QScriptEngine* engine, QObject *parent = 0);

    ~ExposedModel();
    void stateElementModified(model::StateElement *stateElement);
    
signals:
    void elementModified(QString key);
public slots:
    void notifyListeners(QString key);
    void updateElement(const QString& key, QScriptValue value);

    QScriptValue getElementValue(const QString& key);

    void addLocalListener(const QString& key, QScriptValue function);


private:
    QScriptEngine* m_engine;
    boost::shared_ptr<tinia::model::ExposedModel> m_model;

    std::map<std::string, std::vector<QScriptValue > > m_listeners;
};

} // namespace scripting
} // namespace qtcontroller
} // namespace tinia


