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
#include <tinia/qtcontroller/scripting/EventHandler.hpp>
#include <tinia/qtcontroller/scripting/ScriptEngine.hpp>

BOOST_AUTO_TEST_SUITE(EventHandler)

BOOST_AUTO_TEST_CASE(MethodsCalled) {
    auto model = std::make_shared<tinia::model::ExposedModel>();
    model->addElement<int>("moveCalled", 0);
    model->addElement<int>("pressCalled", 0);
    model->addElement<int>("releaseCalled", 0);
    // Make our basic class
    QString script =
            "function TestEventHandler(parameters) {\n"
            "   this.model = parameters.exposedModel;\n"
            "}\n"
            "TestEventHandler.prototype = {\n"
            "   mouseMoveEvent: function(event) {\n"
            "       this.model.updateElement('moveCalled', 1);\n"
            "   },\n"
            "   mousePressEvent: function(event) {\n"
            "       this.model.updateElement('pressCalled', 1);\n"
            "   },\n"
            "   mouseReleaseEvent: function(event) {\n"
            "       this.model.updateElement('releaseCalled', 1);\n"
            "   }\n"
            "}\n";

    auto& engine = tinia::qtcontroller::scripting::ScriptEngine::getInstance()->engine();
    engine.evaluate(script);

    tinia::qtcontroller::scripting::EventHandler handler("TestEventHandler", model);

    // Assert the variables aren't touched yet:
    int moveCalled, releaseCalled, pressCalled;
    model->getElementValue("moveCalled", moveCalled);
    model->getElementValue("releaseCalled", releaseCalled);
    model->getElementValue("pressCalled", pressCalled);

    BOOST_CHECK_EQUAL(0, moveCalled);
    BOOST_CHECK_EQUAL(0, releaseCalled);
    BOOST_CHECK_EQUAL(0, pressCalled);

    QMouseEvent *e = new QMouseEvent(QEvent::MouseMove, QPoint(0,0), QPoint(0,0),
                                     Qt::NoButton, Qt::NoButton, Qt::NoModifier);

    // Now we fire the events
    handler.mouseMoveEvent(e);
    model->getElementValue("moveCalled", moveCalled);
    model->getElementValue("releaseCalled", releaseCalled);
    model->getElementValue("pressCalled", pressCalled);
    BOOST_CHECK_EQUAL(1, moveCalled);
    BOOST_CHECK_EQUAL(0, releaseCalled);
    BOOST_CHECK_EQUAL(0, pressCalled);


    handler.mousePressEvent(e);

    model->getElementValue("moveCalled", moveCalled);
    model->getElementValue("releaseCalled", releaseCalled);
    model->getElementValue("pressCalled", pressCalled);
    BOOST_CHECK_EQUAL(1, moveCalled);
    BOOST_CHECK_EQUAL(0, releaseCalled);
    BOOST_CHECK_EQUAL(1, pressCalled);


    handler.mouseReleaseEvent(e);

    model->getElementValue("moveCalled", moveCalled);
    model->getElementValue("releaseCalled", releaseCalled);
    model->getElementValue("pressCalled", pressCalled);
    BOOST_CHECK_EQUAL(1, moveCalled);
    BOOST_CHECK_EQUAL(1, releaseCalled);
    BOOST_CHECK_EQUAL(1, pressCalled);
}

BOOST_AUTO_TEST_CASE(EventObject) {
    auto model = std::make_shared<tinia::model::ExposedModel>();

    QString script =
            "moveX = -1;\n"
            "moveY = -1;\n"
            "pressX = -1;\n"
            "pressY = -1;\n"
            "releaseX = -1\n"
            "releaseY = -1\n"
            "function MyEventObjectTest(params) {\n"
            "   this.model = params.exposedModel;\n"
            "}\n"
            "MyEventObjectTest.prototype = {\n"
            "   mouseMoveEvent: function(event) {\n"
            "       moveX = event.x;\n"
            "       moveY = event.y;\n"
            "   },\n"
            "   mousePressEvent: function(event) {\n"
            "       pressX = event.x;\n"
            "       pressY = event.y;\n"
            "   },\n"
            "   mouseReleaseEvent: function(event) {\n"
            "       releaseX = event.x;\n"
            "       releaseY = event.y;\n"
            "   }\n"
            "}\n";
    auto& engine = tinia::qtcontroller::scripting::ScriptEngine::getInstance()->engine();

    engine.evaluate(script);

    BOOST_CHECK_EQUAL(-1, engine.evaluate("moveX").toNumber());
    BOOST_CHECK_EQUAL(-1, engine.evaluate("moveY").toNumber());

    BOOST_CHECK_EQUAL(-1, engine.evaluate("pressX").toNumber());
    BOOST_CHECK_EQUAL(-1, engine.evaluate("pressY").toNumber());

    BOOST_CHECK_EQUAL(-1, engine.evaluate("releaseX").toNumber());
    BOOST_CHECK_EQUAL(-1, engine.evaluate("releaseY").toNumber());

    tinia::qtcontroller::scripting::EventHandler handler("MyEventObjectTest",
                                                         model);

    QMouseEvent *e = new QMouseEvent(QEvent::MouseMove, QPoint(42,43), QPoint(0,0),
                                     Qt::NoButton, Qt::NoButton, Qt::NoModifier);

    handler.mouseMoveEvent(e);
    BOOST_CHECK_EQUAL(42, engine.evaluate("moveX").toNumber());
    BOOST_CHECK_EQUAL(43, engine.evaluate("moveY").toNumber());

    BOOST_CHECK_EQUAL(-1, engine.evaluate("pressX").toNumber());
    BOOST_CHECK_EQUAL(-1, engine.evaluate("pressY").toNumber());

    BOOST_CHECK_EQUAL(-1, engine.evaluate("releaseX").toNumber());
    BOOST_CHECK_EQUAL(-1, engine.evaluate("releaseY").toNumber());


    handler.mousePressEvent(e);
    BOOST_CHECK_EQUAL(42, engine.evaluate("moveX").toNumber());
    BOOST_CHECK_EQUAL(43, engine.evaluate("moveY").toNumber());

    BOOST_CHECK_EQUAL(42, engine.evaluate("pressX").toNumber());
    BOOST_CHECK_EQUAL(43, engine.evaluate("pressY").toNumber());

    BOOST_CHECK_EQUAL(-1, engine.evaluate("releaseX").toNumber());
    BOOST_CHECK_EQUAL(-1, engine.evaluate("releaseY").toNumber());


    handler.mouseReleaseEvent(e);
    BOOST_CHECK_EQUAL(42, engine.evaluate("moveX").toNumber());
    BOOST_CHECK_EQUAL(43, engine.evaluate("moveY").toNumber());

    BOOST_CHECK_EQUAL(42, engine.evaluate("pressX").toNumber());
    BOOST_CHECK_EQUAL(43, engine.evaluate("pressY").toNumber());

    BOOST_CHECK_EQUAL(42, engine.evaluate("releaseX").toNumber());
    BOOST_CHECK_EQUAL(43, engine.evaluate("releaseY").toNumber());
}

BOOST_AUTO_TEST_SUITE_END()



