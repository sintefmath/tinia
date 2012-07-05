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

#define BOOST_TEST_ALTERNATIVE_INIT_API

#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <QFile>
#include <QtScript>
#include <QCoreApplication>

#include <stdexcept>

namespace {

using namespace std;

using namespace boost::unit_test;
struct SetupQtApplication {
    QCoreApplication app;

    SetupQtApplication()
        : app( framework::master_test_suite().argc, framework::master_test_suite().argv )
    {
        ;
    }
};

struct BringUpJavaScript {
    QScriptEngine engine;
    QScriptValue object;

    BringUpJavaScript( QString fileName ) {
        QFile scriptFile( fileName );
        BOOST_ASSERT( scriptFile.open( QIODevice::ReadOnly ) );
        BOOST_ASSERT( true );

        QTextStream stream(&scriptFile);
        auto contents = stream.readAll();
        scriptFile.close();
        object = engine.evaluate( contents, fileName );

        /*auto func = object.property( "foo" );

        auto lineEditVal = engine.newQObject( ui->lineEdit );
       // auto result = func.call( object, QScriptValueList() << lineEditVal );
       */
    }
};

void testJs( QString fileName ) {
    QScriptEngine engine;

    QFile scriptFile( fileName );
    BOOST_ASSERT( scriptFile.open( QIODevice::ReadOnly ) );

    QTextStream stream(&scriptFile);
    auto contents = stream.readAll();
    scriptFile.close();

    auto object = engine.evaluate( contents, fileName );
    auto result1 = engine.evaluate("vec3.createFrom(1.123456789123456,2,3)");


    //auto r = engine.fromScriptValue<QObjectList>(result1);
    BOOST_TEST_MESSAGE(result1.property(0).toNumber());
    std::stringstream ss;
             ss.precision(16);
    ss << result1.property(0).toNumber();
    BOOST_TEST_MESSAGE("FROM SS: " + ss.str());
    BOOST_TEST_MESSAGE("FROM JS: " + result1.toString().toStdString());
    //BOOST_TEST_MESSAGE( ((QList<double>*)result1.toQObject())->at(0) );
}

//BOOST_AUTO_TEST_CASE( javascriptinterop )
//{
//    BOOST_TEST_MESSAGE( "Hello" );
//    BOOST_ASSERT( false );
//}

bool
init_unit_test_suite() {
    QString params[] = { ":orig/gl-matrix.js", ":orig/gl-matrix-min.js", ":ours/glMatrix.js" };
    boost::unit_test::framework::master_test_suite().add( BOOST_PARAM_TEST_CASE( &testJs, begin(params), end(params) ) );

    return true;
}

}

BOOST_GLOBAL_FIXTURE( SetupQtApplication );

int
main( int argc, char* argv[] )
{
    return ::boost::unit_test::unit_test_main( &init_unit_test_suite, argc, argv );
}

