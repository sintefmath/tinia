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

// QT's moc doesn't like BOOST_JOIN ( can be removed in QT 5.0 we think)
#ifndef Q_MOC_RUN 
#include <boost/test/unit_test.hpp>
#endif
#include "tinia/qtcontroller/impl/http_utils.hpp"
#include <QString>

BOOST_AUTO_TEST_SUITE(QtServer)

BOOST_AUTO_TEST_CASE(IsGet) {
    QString uri ="uriFilename.ext";
    QString request = "GET " + uri + " HTTP/1.1\n";
    BOOST_CHECK(tinia::qtcontroller::impl::isGetOrPost(request));

    QString notGet ="NotGET bla";
    BOOST_CHECK(!tinia::qtcontroller::impl::isGetOrPost(notGet));
}

BOOST_AUTO_TEST_CASE(DecodeGetUri) {
    QString uri ="uriFilename.ext";
    QString request = "GET " + uri + " HTTP/1.1\n";
    BOOST_CHECK_EQUAL(uri.toStdString(),
                      tinia::qtcontroller::impl::getRequestURI(request).toStdString());
}

BOOST_AUTO_TEST_CASE(DecodeGetUriThrow) {
    QString uri ="uriFilename.ext";
    QString request = "OPTIONS " + uri + " HTTP/1.1\n";
    BOOST_CHECK_THROW(tinia::qtcontroller::impl::getRequestURI(request),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(GetParams)  {
    QString request = "GET filename.txt?key1=val&key2&key3=val2";

    QMap<QString, QString> params = tinia::qtcontroller::impl::decodeGetParameters(request);
    BOOST_CHECK_EQUAL(3, params.size());
    BOOST_CHECK_EQUAL(std::string("val"), params["key1"].toStdString());
    BOOST_CHECK(params.contains("key2"));
    BOOST_CHECK_EQUAL(std::string(""), params["key2"].toStdString());
    BOOST_CHECK_EQUAL(std::string("val2"), params["key3"].toStdString());
}

BOOST_AUTO_TEST_CASE(ParseGet) {

    QMap<QString, QString> params;
    params["key"] = "value";
    params["uintKey"] = "42";
    params["intKey"] = "-43";
    boost::tuple<std::string, unsigned int, int> args = tinia::qtcontroller::impl::parseGet<boost::tuple<std::string, unsigned int, int>>(params, "key uintKey intKey");
    BOOST_CHECK_EQUAL("value", args.get<0>());
    BOOST_CHECK_EQUAL(42, args.get<1>());
    BOOST_CHECK_EQUAL(-43, args.get<2>());

    BOOST_CHECK_THROW(tinia::qtcontroller::impl::parseGet<boost::tuple<unsigned int>>(params, "keyNotInThere"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(PostContent) {
    QString content = "POST /login.jsp HTTP/1.1\r\n"
            "Host: www.mysite.com\r\n"
            "User-Agent: Mozilla/4.0\r\n"
            "Content-Length: 27\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "\r\n"
            "CONTENT HERE";
    BOOST_CHECK_EQUAL("CONTENT HERE", tinia::qtcontroller::impl::getPostContent(content).toStdString());
}

BOOST_AUTO_TEST_SUITE_END()
