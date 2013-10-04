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

#include <boost/property_tree/ptree.hpp>
#include "tinia/model/impl/ElementData.hpp"
using namespace tinia;
BOOST_AUTO_TEST_SUITE( ElementData )

BOOST_AUTO_TEST_CASE( gettersAndSetters ) {
    model::impl::ElementData ed;

    std::string myString( "ipsum" );
    std::string myType( "xsd:string" );

    ed.setStringValue( myString );
    BOOST_CHECK_EQUAL( ed.getStringValue(), myString );

    ed.setXSDType( myType );
    BOOST_CHECK_EQUAL( ed.getXSDType(), myType );

    ed.setMinConstraint( "0" );
    BOOST_CHECK_EQUAL( ed.getMinConstraint(), "0" );

    ed.setMaxConstraint( "42" );
    BOOST_CHECK_EQUAL( ed.getMaxConstraint(), "88" );

    std::set<std::string> restrictionSet;
	restrictionSet.insert("wireframe");
	restrictionSet.insert("solid");
    ed.setRestrictionSet( restrictionSet );
    BOOST_CHECK( restrictionSet == ed.getEnumerationSet() );

	std::map<std::string, std::string> annotationMap;
	annotationMap["en"]= "strawberry jam";

    ed.setAnnotation( annotationMap );
    BOOST_CHECK( annotationMap == ed.getAnnotation() );

    BOOST_CHECK_EQUAL( ed.getLength(), ed.LENGTH_NOT_SET );
    ed.setLength( 16 );
    BOOST_CHECK_EQUAL( ed.getLength(), 16 );
}

BOOST_AUTO_TEST_CASE( emptyConstraints ) {
    model::impl::ElementData ed;

    BOOST_CHECK( ed.emptyConstraints() );
}

BOOST_AUTO_TEST_CASE( emptyRestrictions ) {
    model::impl::ElementData ed;

    BOOST_CHECK( ed.emptyRestrictionSet() );
}

BOOST_AUTO_TEST_CASE( hasAnnotation ) {
    model::impl::ElementData ed;

    BOOST_CHECK( ed.emptyAnnotation() );
}

BOOST_AUTO_TEST_CASE( propertyTree ) {
    model::impl::ElementData ed;

    //typedef boost::property_tree::basic_ptree<std::string, model::impl::ElementData> PropertyTree;
    model::impl::ElementData::PropertyTree& pt = ed.getPropertyTree();
    BOOST_CHECK( pt.empty() );
}






BOOST_AUTO_TEST_SUITE_END()
