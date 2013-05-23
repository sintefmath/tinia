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

#include <boost/test/unit_test.hpp>
#include "tinia/qtcontroller/impl/synchronous_http_get.hpp"
#include "tinia/qtcontroller/moc/HTTPServer.hpp"
#include "tinia/qtcontroller/moc/OpenGLServerGrabber.hpp"
#include <QFile>
#include <QRunnable>
#include <QThreadPool>
#include <QEventLoop>

namespace {
    struct TestJob : public tinia::jobcontroller::OpenGLJob {
        bool renderFrame(const std::string &session, const std::string &key, unsigned int fbo, const size_t width, const size_t height) {
            // Do nothing
            return true;
        }
    };
}
BOOST_AUTO_TEST_SUITE(SynchronousGetTest)

BOOST_AUTO_TEST_CASE(GetSomeStaticJavaScriptFile) {
    TestJob job;

    // We need one instance of these:
    BOOST_MESSAGE("Constructing server grabber");
    tinia::qtcontroller::impl::OpenGLServerGrabber* serverGrabber =
            new tinia::qtcontroller::impl::OpenGLServerGrabber(&job);
    // Start server
    BOOST_MESSAGE("Constructing server");
    tinia::qtcontroller::impl::HTTPServer server(&job, serverGrabber, NULL);


    QUrl indexURL(QString("http://localhost:8080/index.html"));

    BOOST_MESSAGE("Getting reply from server");
    QNetworkReply* reply = tinia::qtcontroller::impl::getSynchronousHTTP(indexURL);

    // Storing these as std::string for easy comparison in boost-test later
    std::string replyData = (QString((reply->readAll()))).trimmed().toStdString();

    BOOST_MESSAGE("Opening file.");
    // Open the same internal file
    QFile file(":/javascript/index.html");
    if (!file.open(QIODevice::ReadOnly)) {
        BOOST_FAIL("Could not open QResource file :/javascript/index.html");
    }


    BOOST_MESSAGE("Read both files, comparing data");
    std::string expectedData = QString(file.readAll()).trimmed().toStdString();

    BOOST_CHECK_EQUAL(expectedData, replyData);
}

BOOST_AUTO_TEST_SUITE_END()

