#include <boost/test/unit_test.hpp>

#include <boost/property_tree/ptree.hpp>
#include "tinia/policy/ElementData.hpp"
using namespace tinia;
BOOST_AUTO_TEST_SUITE( ElementData )

BOOST_AUTO_TEST_CASE( gettersAndSetters ) {
    policy::ElementData ed;

    std::string myString( "ipsum" );
    std::string myType( "xsd:string" );

    ed.setStringValue( myString );
    BOOST_CHECK_EQUAL( ed.getStringValue(), myString );

    ed.setXSDType( myType );
    BOOST_CHECK_EQUAL( ed.getXSDType(), myType );

    ed.setMinConstraint( "0" );
    BOOST_CHECK_EQUAL( ed.getMinConstraint(), "0" );

    ed.setMaxConstraint( "42" );
    BOOST_CHECK_EQUAL( ed.getMaxConstraint(), "42" );

    std::unordered_set<std::string> restrictionSet;
	restrictionSet.insert("wireframe");
	restrictionSet.insert("solid");
    ed.setRestrictionSet( restrictionSet );
    BOOST_CHECK( restrictionSet == ed.getEnumerationSet() );

	std::unordered_map<std::string, std::string> annotationMap;
	annotationMap["en"]= "strawberry jam";

    ed.setAnnotation( annotationMap );
    BOOST_CHECK( annotationMap == ed.getAnnotation() );

    BOOST_CHECK_EQUAL( ed.getLength(), ed.LENGTH_NOT_SET );
    ed.setLength( 16 );
    BOOST_CHECK_EQUAL( ed.getLength(), 16 );
}

BOOST_AUTO_TEST_CASE( emptyConstraints ) {
    policy::ElementData ed;

    BOOST_CHECK( ed.emptyConstraints() );
}

BOOST_AUTO_TEST_CASE( emptyRestrictions ) {
    policy::ElementData ed;

    BOOST_CHECK( ed.emptyRestrictionSet() );
}

BOOST_AUTO_TEST_CASE( hasAnnotation ) {
    policy::ElementData ed;

    BOOST_CHECK( ed.emptyAnnotation() );
}

BOOST_AUTO_TEST_CASE( propertyTree ) {
    policy::ElementData ed;

    //typedef boost::property_tree::basic_ptree<std::string, policy::ElementData> PropertyTree;
    auto& pt = ed.getPropertyTree();
    BOOST_CHECK( pt.empty() );
}






BOOST_AUTO_TEST_SUITE_END()
