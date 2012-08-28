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
#include "tinia/qtcontroller/impl/http_utils.hpp"
#include <QString>

BOOST_AUTO_TEST_SUITE(QtServer)

BOOST_AUTO_TEST_CASE(IsGet) {
    QString uri ="uriFilename.ext";
    QString request = "GET " + uri + " HTTP/1.1\n";
    BOOST_CHECK(tinia::qtcontroller::impl::isGet(request));

    QString notGet ="NotGET bla";
    BOOST_CHECK(!tinia::qtcontroller::impl::isGet(notGet));
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

    auto params = tinia::qtcontroller::impl::decodeGetParameters(request);
    BOOST_CHECK_EQUAL(3, params.size());
    BOOST_CHECK_EQUAL(std::string("val"), params["key1"].toStdString());
    BOOST_CHECK(params.contains("key2"));
    BOOST_CHECK_EQUAL(std::string(""), params["key2"].toStdString());
    BOOST_CHECK_EQUAL(std::string("val2"), params["key3"].toStdString());
}

BOOST_AUTO_TEST_SUITE_END()
