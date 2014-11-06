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
    if(!isGetOrPost(request)) {
        throw std::invalid_argument("Request line is not a GET request.");
    }
    return request.split(QRegExp("[ \r\n][ \r\n]*"))[1].split('?')[0];
}
bool isGetOrPost(const QString& request) {
    QString method = request.split(QRegExp("[ \r\n][ \r\n]*"))[0];
    return method == "GET" || method == "POST";
}

QString getMimeType(const QString& file) {
    QString extension = file.split('.').last();

    QMap<QString, QString> extensions;

    extensions["js"] = "application/javascript";
    extensions["html"] = "text/html";
    extensions["txt"] = "text/plain";
    extensions["xml"] = "application/xml";
    extensions["css"] = "text/css";
    extensions["bin"] = "application/octet-stream";

    return extensions[extension];
}

QMap<QString, QString> decodeGetParameters(const QString& request) {
    QMap<QString, QString> keyValue;
    QStringList split1 = request.split(QRegExp("[ \r\n][ \r\n]*"));
    if(split1.size() < 2) {
        return keyValue;
    }

    QStringList split2 = split1[1].split('?');
    if(split2.size() < 2) {
        return keyValue;
    }
    QStringList params = split2[1].split('&');
    for(int i = 0; i < params.size(); ++i) {
        QStringList split = params[i].split('=');
        if(split.size()==1) {
            keyValue[split[0]] = "";
        } else if(split.size() > 1){
            keyValue[split[0]] = split[1];
        }

    }
    return keyValue;
}

QString getPostContent(const QString& request) {
    return request.mid(request.indexOf(QRegExp("\r\n[ ]*\r\n"))).trimmed();
}

QString httpHeader(const QString& mime, unsigned int code, const QString& encoding, const unsigned int size) {
    QString result = "HTTP/1.1 " + QString::number(code) + " OK\r\n" +
            + "Content-Type: " + mime + "; charset=\"" + encoding + "\"\r\n";
    if ( size != 0 ) {
        result += "Content-Length: " + QString::number(size) + "\r\n";
    }
    return result;
}



}}}
