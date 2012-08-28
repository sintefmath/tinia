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

#include "tinia/qtcontroller/moc/HTTPServer.hpp"
#include "tinia/qtcontroller/impl/http_utils.hpp"
#include <QStringList>
#include <QDateTime>

namespace tinia {
namespace qtcontroller {
namespace impl {

HTTPServer::HTTPServer(QObject *parent) :
    QTcpServer(parent)
{
    listen(QHostAddress::Any, 8080);
    qDebug("Started");
}

void HTTPServer::incomingConnection(int socket)
{
    qDebug("HTTPServer: request");

    QTcpSocket* s = new QTcpSocket(this);
    connect(s, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
    s->setSocketDescriptor(socket);

}

void HTTPServer::readyRead()
{
    qDebug("HTTPServer: receiving");
    QTcpSocket* socket = (QTcpSocket*)sender();
    if (socket->canReadLine()) {
        auto line = QString(socket->readLine());
        if (isGet(line)) {
            QTextStream os(socket);
            os.setAutoDetectUnicode(true);
            os << "HTTP/1.1 200 Ok\r\n"
                  "Content-Type: text/html; charset=\"utf-8\"\r\n"
                  "\r\n"
                  "<h1>Nothing to see here</h1>\n<hr />\n"
               << "You asked for: <b>"<<getRequestURI(line)<<"</b><br />"
                  << line;


            while(socket->canReadLine()) {
                auto line = QString(socket->readLine());
                os << line<<"<br />\n";
            }

            os<< QDateTime::currentDateTime().toString() << "\n";
            socket->close();


            if (socket->state() == QTcpSocket::UnconnectedState) {
                delete socket;
            }
        }
    }

}

void HTTPServer::discardClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();

}

}
} // namespace qtcontroller
} // namespace tinia
