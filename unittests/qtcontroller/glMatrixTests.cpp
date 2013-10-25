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
#include <QFile>
#include <QtScript>

#include <stdexcept>
#include <tinia/qtcontroller/impl/script_utils.hpp>
namespace {

using namespace std;

using namespace boost::unit_test;

BOOST_AUTO_TEST_CASE( TestIncludeGlMatrix ) {

    QScriptEngine engine;
    tinia::qtcontroller::scripting::addDefaultScripts(engine);

    // This should now work:
    BOOST_CHECK_EQUAL(1,
        engine.evaluate("((vec3.createFrom(0,1,2))[1])").toNumber());

    // Check that we have the basic types:
    BOOST_CHECK(engine.evaluate("vec3").isObject());
    BOOST_CHECK(engine.evaluate("vec3.createFrom").isFunction());
    BOOST_CHECK(engine.evaluate("vec3.negate").isFunction());
    BOOST_CHECK(engine.evaluate("mat4").isObject());
    BOOST_CHECK(engine.evaluate("quat4").isObject());
    BOOST_CHECK(engine.evaluate("vec4").isObject());
    BOOST_CHECK(engine.evaluate("vec2").isObject());
}

BOOST_AUTO_TEST_CASE( TestVec3Add ) {
    QScriptEngine engine;
    tinia::qtcontroller::scripting::addDefaultScripts(engine);
    QString script = "(function() {"
            "var a = vec3.createFrom(0, 1, 2);"
            "var b = vec3.createFrom(2, 1, 0);"
            "var c = vec3.create();"
            "return vec3.add(a, b, c);"
            "})";
    QScriptValue returnValue = engine.evaluate(script).call(QScriptValue());
    BOOST_CHECK_EQUAL(2, returnValue.property(0).toNumber());
    BOOST_CHECK_EQUAL(2, returnValue.property(1).toNumber());
    BOOST_CHECK_EQUAL(2, returnValue.property(2).toNumber());
}

BOOST_AUTO_TEST_CASE( TestVec3Subtract ) {
    QScriptEngine engine;
    tinia::qtcontroller::scripting::addDefaultScripts(engine);
    QString script = "(function() {"
            "var a = vec3.createFrom(0, 1, 2);"
            "var b = vec3.createFrom(2, 1, 0);"
            "var c = vec3.create();"
            "return vec3.subtract(a, b, c);"
            "})";
    QScriptValue returnValue = engine.evaluate(script).call(QScriptValue());
    BOOST_CHECK_EQUAL(-2, returnValue.property(0).toNumber());
    BOOST_CHECK_EQUAL(0, returnValue.property(1).toNumber());
    BOOST_CHECK_EQUAL(2, returnValue.property(2).toNumber());
}

BOOST_AUTO_TEST_CASE( TestMat4MultiplyWithVec4) {
    QScriptEngine engine;
    tinia::qtcontroller::scripting::addDefaultScripts(engine);
    QString script = "(function() {"
            "var x = vec3.createFrom(1, 1, 1);"
            "var A = mat4.createFrom(2, 0, 0, 0,"
            "                        0, 2, 0, 0,"
            "                        0, 0, 2, 0,"
            "                        0, 0, 0, 1);"
            "return mat4.multiplyVec3(A, x);"
            "})";

    QScriptValue returnValue = engine.evaluate(script).call(QScriptValue());
    BOOST_CHECK_EQUAL(2, returnValue.property(0).toNumber());
    BOOST_CHECK_EQUAL(2, returnValue.property(1).toNumber());
    BOOST_CHECK_EQUAL(2, returnValue.property(2).toNumber());
}
}

