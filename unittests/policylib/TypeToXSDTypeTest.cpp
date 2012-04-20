#include <boost/test/unit_test.hpp>
#include "tinia/policylib/TypeToXSDType.hpp"

BOOST_AUTO_TEST_SUITE( TypeToXSDType )

BOOST_AUTO_TEST_CASE( intMapping ) {
    std::string typeString = policylib::TypeToXSDType<int>::getTypename();
    BOOST_CHECK_EQUAL( typeString, "xsd:integer" );
}

BOOST_AUTO_TEST_CASE( floatMapping ) {
    std::string typeString = policylib::TypeToXSDType<float>::getTypename();
    BOOST_CHECK_EQUAL( typeString, "xsd:float" );
}

BOOST_AUTO_TEST_CASE( doubleMapping ) {
    std::string typeString = policylib::TypeToXSDType<double>::getTypename();
    BOOST_CHECK_EQUAL( typeString, "xsd:double" );
}

BOOST_AUTO_TEST_CASE( stringMapping ) {
    std::string typeString = policylib::TypeToXSDType<std::string>::getTypename();
    BOOST_CHECK_EQUAL( typeString, "xsd:string" );
}

BOOST_AUTO_TEST_CASE( charpointerMapping ) {
    std::string typeString = policylib::TypeToXSDType<char*>::getTypename();
    BOOST_CHECK_EQUAL( typeString, "xsd:string" );
}

BOOST_AUTO_TEST_CASE( constCharpointerMapping ) {
    std::string typeString = policylib::TypeToXSDType<const char*>::getTypename();
    BOOST_CHECK_EQUAL( typeString, "xsd:string" );
}

BOOST_AUTO_TEST_SUITE_END()
