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
#include <tinia/qtcontroller/scripting/ScriptEngine.hpp>

BOOST_AUTO_TEST_SUITE(ScriptEngine)

BOOST_AUTO_TEST_CASE(TestUniqueness) {
    auto& a = tinia::qtcontroller::scripting::scriptEngineInstance();
    auto& b = tinia::qtcontroller::scripting::scriptEngineInstance();
    BOOST_CHECK_EQUAL(&a, &b);
}

BOOST_AUTO_TEST_CASE(TestScriptAdd) {
    auto& engine = tinia::qtcontroller::scripting::scriptEngineInstance();
    engine.evaluate("function functionAddedInThisTest() { return 42; }");

    BOOST_CHECK(engine.evaluate("functionAddedInThisTest").isFunction());
    BOOST_CHECK_EQUAL(42, engine.evaluate("functionAddedInThisTest()").toNumber());

    auto& otherEngine = tinia::qtcontroller::scripting::scriptEngineInstance();

    BOOST_CHECK(otherEngine.evaluate("functionAddedInThisTest").isFunction());
    BOOST_CHECK_EQUAL(42, otherEngine.evaluate("functionAddedInThisTest()").toNumber());
}

BOOST_AUTO_TEST_SUITE_END()

