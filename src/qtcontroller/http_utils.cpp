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

#include "tinia/qtcontroller/impl/http_utils.hpp"
#include <QRegExp>
#include <QStringList>
#include <stdexcept>
#include <QMap>

namespace tinia { namespace qtcontroller { namespace impl {
QString getRequestURI(const QString& request) {
    if(!isGet(request)) {
        throw std::invalid_argument("Request line is not a GET request.");
    }
    return request.split(QRegExp("[ \r\n][ \r\n]*"))[1].split('?')[0];
}
bool isGet(const QString& request) {
    return request.split(QRegExp("[ \r\n][ \r\n]*"))[0] == "GET";
}

QString getMimeType(const QString& file) {
    auto extension = file.split('.').last();

    QMap<QString, QString> extensions;

    extensions["js"] = "application/javascript";
    extensions["html"] = "text/html";
    extensions["txt"] = "text/plain";
    extensions["xml"] = "application/xml";
    extensions["css"] = "text/css";

    return extensions[extension];
}

QMap<QString, QString> decodeGetParameters(const QString& request) {
    auto params = request.split('?')[1].split('&');
    QMap<QString, QString> keyValue;
    for(auto i = 0u; i < params.size(); ++i) {
        auto split = params[i].split('=');
        if(split.size()==1) {
            keyValue[split[0]] = "";
        } else if(split.size() > 1){
            keyValue[split[0]] = split[1];
        }

    }
    return keyValue;
}

}}}
