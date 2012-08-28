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
#include <stdexcept>
#include <iostream>
#include <QStringList>
#include <QDateTime>
#include <QFile>

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
        std::cout << line.toStdString() << std::endl;
        if (isGet(line)) {
            QTextStream os(socket);
            os.setAutoDetectUnicode(true);

            try {
                os << getStaticContent(getRequestURI(line)) <<"\n";
            } catch(std::invalid_argument& e) {
                os << "HTTP/1.1 404 Not Found\r\n"
                      "Content-Type: text/html; charset=\"utf-8\"\r\n"
                      "\r\n";
                  os << "<h1>Nothing to see here</h1>\n<hr />\n"
                    << "You asked for: <b>"<<getRequestURI(line)<<"</b><br />"
                  << line;

                  while(socket->canReadLine()) {
                      auto line = QString(socket->readLine());
                      os << line<<"<br />\n";
                  }

                  os<< QDateTime::currentDateTime().toString() << "\n";
                  std::cout << "Not found: " << line.toStdString() << std::endl;
            }




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

QString HTTPServer::getStaticContent(const QString &uri)
{
    QFile file(":/javascript" + uri);
    if(file.open(QIODevice::ReadOnly)) {
        QString reply =QString("HTTP/1.0 200 Ok\r\n") +
                QString("Content-Type: ") + getMimeType(uri) +
                QString("; charset=\"utf-8\"\r\n \n\n") + file.readAll() + "\n";
        return reply;
    }
    else {
        throw std::invalid_argument("File not found: " + uri.toStdString());
    }
}

}
} // namespace qtcontroller
} // namespace tinia
