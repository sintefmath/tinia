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
#include "tinia/policy/TypeToXSDType.hpp"
using namespace tinia;
BOOST_AUTO_TEST_SUITE( TypeToXSDType )

BOOST_AUTO_TEST_CASE( intMapping ) {
    std::string typeString = policy::TypeToXSDType<int>::getTypename();
    BOOST_CHECK_EQUAL( typeString, "xsd:integer" );
}

BOOST_AUTO_TEST_CASE( floatMapping ) {
    std::string typeString = policy::TypeToXSDType<float>::getTypename();
    BOOST_CHECK_EQUAL( typeString, "xsd:float" );
}

BOOST_AUTO_TEST_CASE( doubleMapping ) {
    std::string typeString = policy::TypeToXSDType<double>::getTypename();
    BOOST_CHECK_EQUAL( typeString, "xsd:double" );
}

BOOST_AUTO_TEST_CASE( stringMapping ) {
    std::string typeString = policy::TypeToXSDType<std::string>::getTypename();
    BOOST_CHECK_EQUAL( typeString, "xsd:string" );
}

BOOST_AUTO_TEST_CASE( charpointerMapping ) {
    std::string typeString = policy::TypeToXSDType<char*>::getTypename();
    BOOST_CHECK_EQUAL( typeString, "xsd:string" );
}

BOOST_AUTO_TEST_CASE( constCharpointerMapping ) {
    std::string typeString = policy::TypeToXSDType<const char*>::getTypename();
    BOOST_CHECK_EQUAL( typeString, "xsd:string" );
}

BOOST_AUTO_TEST_SUITE_END()
