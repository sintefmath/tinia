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

#include <QImage>
#include <QBuffer>
#include <QByteArray>
#include <GL/glew.h>
#include "tinia/qtcontroller/moc/HTTPServer.hpp"
#include "tinia/qtcontroller/moc/LongPollHandler.hpp"
#include "tinia/qtcontroller/impl/ServerThread.hpp"
#include "tinia/qtcontroller/impl/http_utils.hpp"
#include "tinia/renderlist.hpp"
#include <stdexcept>
#include <iostream>
#include <QStringList>
#include <QDateTime>
#include <QFile>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <QThreadPool>

namespace tinia {
namespace qtcontroller {
namespace impl {

HTTPServer::HTTPServer(tinia::jobcontroller::Job* job, tinia::qtcontroller::ImageSource* imageSource, QObject *parent) :
    QTcpServer(parent), m_job(job),
    m_serverGrabber(imageSource)
{
    listen(QHostAddress::Any, 8080);
}

void HTTPServer::incomingConnection(int socket)
{
    ServerThread* thread = new ServerThread(*m_serverGrabber, m_job, socket);

    //connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    QThreadPool::globalInstance()->start(thread);
}

}
} // namespace qtcontroller
} // namespace tinia
