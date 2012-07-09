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

#include <tinia/qtcontroller/scripting/utils.hpp>
#include <QFile>
#include <stdexcept>


namespace tinia { namespace qtcontroller { namespace scripting {

void addDefaultScripts(QScriptEngine &engine) {
    QFile textList(":javascript/shared/includes.txt");
    if(!textList.open(QFile::ReadOnly)) {
        throw std::runtime_error("Could not open the includes file for javascript.");
    }

    while(!textList.atEnd()) {
        QString line = textList.readLine().trimmed();
        if(line.size() == 0)
            continue;
        QFile javascriptFile(":javascript/shared/" + line);

        if(!javascriptFile.open(QFile::ReadOnly)) {
            throw std::runtime_error("Could not open javascript file: " + line.toStdString());
        }

        auto error = engine.evaluate(javascriptFile.readAll());
        if(error.isError()) {
            throw std::runtime_error(error.toString().toStdString());
        }
    }
}
}}}
