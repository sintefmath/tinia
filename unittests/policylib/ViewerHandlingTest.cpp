#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <type_traits>

#include "tinia/policylib/PolicyLib.hpp"
#include "tinia/policylib/Viewer.hpp"
#include "tinia/policylib/utils.hpp"
#include "testutils.hpp"

using namespace std;
using namespace tinia;
using namespace policylib;

BOOST_AUTO_TEST_SUITE( ViewerHandlingTest )

struct ViewerFixture {
    ViewerFixture() :
        xsd( "http://www.w3.org/2001/XMLSchema" ),
        xsi( "http://www.w3.org/2001/XMLSchema-instance" ),
        tns( "http://cloudviz.sintef.no/V1/policy" )
    {

    }
    ~ViewerFixture() {}

    policylib::PolicyLib policyLib;
    policylib::Viewer viewer;
    const std::string xsd;
    const std::string xsi;
    const std::string tns;
};

BOOST_FIXTURE_TEST_CASE( addgetViewer, ViewerFixture ) {
    viewer.height += 1;
    viewer.width += 1;
    viewer.timestamp += 100.0;

    fill( viewer.projectionMatrix.begin(), viewer.projectionMatrix.end(), 3.14f );
    fill( viewer.modelviewMatrix.begin(), viewer.modelviewMatrix.end(), 1.0f );

    policyLib.addElement( "viewer1", viewer );
    policyLib.addElement( "anInt", 1 );

    Viewer readViewer;

    BOOST_CHECK_THROW( policyLib.getElementValue( "anInt", readViewer ), std::runtime_error );

    policyLib.getElementValue( "viewer1", readViewer );
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
    policyLib.addElement( "viewer1", viewer );

    viewer.height = 2 * viewer.height;

    //policyLib.updateElement( "viewer1", viewer );

    Viewer readViewer;
    policyLib.getElementValue( "viewer1", readViewer );
    BOOST_CHECK_EQUAL( readViewer.height, viewer.height );
}



BOOST_AUTO_TEST_SUITE_END()
