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

#include <algorithm>
#include <type_traits>

#include "tinia/model/ExposedModel.hpp"
#include "tinia/model/Viewer.hpp"
#include "tinia/model/impl/utils.hpp"
#include "testutils.hpp"

using namespace std;
using namespace tinia;
using namespace model;

BOOST_AUTO_TEST_SUITE( ViewerHandlingTest )
namespace {
struct ViewerFixture {
    ViewerFixture() :
        xsd( "http://www.w3.org/2001/XMLSchema" ),
        xsi( "http://www.w3.org/2001/XMLSchema-instance" ),
        tns( "http://cloudviz.sintef.no/V1/model" )
    {

    }
    ~ViewerFixture() {}

    model::ExposedModel model;
    model::Viewer viewer;
    const std::string xsd;
    const std::string xsi;
    const std::string tns;
};
}

BOOST_FIXTURE_TEST_CASE( addgetViewer, ViewerFixture ) {
    viewer.height += 1;
    viewer.width += 1;
    viewer.timestamp += 100.0;

    fill( viewer.projectionMatrix.begin(), viewer.projectionMatrix.end(), 3.14f );
    fill( viewer.modelviewMatrix.begin(), viewer.modelviewMatrix.end(), 1.0f );

    model.addElement( "viewer1", viewer );
    model.addElement( "anInt", 1 );

    Viewer readViewer;

    BOOST_CHECK_THROW( model.getElementValue( "anInt", readViewer ), std::runtime_error );

    model.getElementValue( "viewer1", readViewer );
    BOOST_CHECK_EQUAL( viewer.height, readViewer.height );
    BOOST_CHECK_EQUAL( viewer.width, readViewer.width );
    BOOST_CHECK_EQUAL( viewer.timestamp, readViewer.timestamp );

    BOOST_CHECK_EQUAL_COLLECTIONS( viewer.projectionMatrix.begin(), viewer.projectionMatrix.end(),
                                   readViewer.projectionMatrix.begin(), readViewer.projectionMatrix.end() );

    BOOST_CHECK_EQUAL_COLLECTIONS( viewer.modelviewMatrix.begin(), viewer.modelviewMatrix.end(),
                                   readViewer.modelviewMatrix.begin(), readViewer.modelviewMatrix.end() );

}




BOOST_FIXTURE_TEST_CASE( validateTypeTraits, ViewerFixture ) {
    BOOST_CHECK( std::is_class<Viewer>::value );
    BOOST_CHECK( !std::is_class<int>::value );
}

BOOST_FIXTURE_TEST_CASE( updateViewer, ViewerFixture ) {
    model.addElement( "viewer1", viewer );

    viewer.height = 2 * viewer.height;

    //model.updateElement( "viewer1", viewer );

    Viewer readViewer;
    model.getElementValue( "viewer1", readViewer );
    BOOST_CHECK_EQUAL( readViewer.height, viewer.height );
}



BOOST_AUTO_TEST_SUITE_END()
