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
