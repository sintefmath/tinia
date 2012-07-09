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

    tinia::qtcontroller::scripting::EventHandler handler("TestEventHandler", "key", model, engine);

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

    delete e;
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

    tinia::qtcontroller::scripting::EventHandler handler("MyEventObjectTest", "key",
                                                         model, engine);

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

    delete e;
}

BOOST_AUTO_TEST_CASE(ButtonsTest) {
    auto model = std::make_shared<tinia::model::ExposedModel>();

    QString script =
            "buttonMove = -2;\n"
            "buttonPress = -2;\n"
            "buttonRelease = -2;\n"
            "function MyButtonTest(params) {\n"
            "   this.model = params.exposedModel;\n"
            "}\n"
            "MyButtonTest.prototype = {\n"
            "   mouseMoveEvent: function(event) {\n"
            "       buttonMove = event.button;\n"
            "   },\n"
            "   mousePressEvent: function(event) {\n"
            "       buttonPress = event.button;\n"
            "   },\n"
            "   mouseReleaseEvent: function(event) {\n"
            "       buttonRelease = event.button;\n"
            "   }\n"
            "}\n";

    auto& engine = tinia::qtcontroller::scripting::ScriptEngine::getInstance()->engine();

    engine.evaluate(script);

    BOOST_CHECK_EQUAL(-2, engine.evaluate("buttonMove").toNumber());
    BOOST_CHECK_EQUAL(-2, engine.evaluate("buttonPress").toNumber());
    BOOST_CHECK_EQUAL(-2, engine.evaluate("buttonRelease").toNumber());

    tinia::qtcontroller::scripting::EventHandler handler("MyButtonTest", "key", model, engine);

    QMouseEvent eMove(QEvent::MouseMove, QPoint(42,43), QPoint(0,0),
                  Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    handler.mouseMoveEvent(&eMove);
    BOOST_CHECK_EQUAL(-1, engine.evaluate("buttonMove").toNumber());
    BOOST_CHECK_EQUAL(-2, engine.evaluate("buttonPress").toNumber());
    BOOST_CHECK_EQUAL(-2, engine.evaluate("buttonRelease").toNumber());


    QMouseEvent ePress(QEvent::MouseMove, QPoint(42,43), QPoint(0,0),
                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    handler.mousePressEvent(&ePress);
    BOOST_CHECK_EQUAL(-1, engine.evaluate("buttonMove").toNumber());
    BOOST_CHECK_EQUAL(0, engine.evaluate("buttonPress").toNumber());
    BOOST_CHECK_EQUAL(-2, engine.evaluate("buttonRelease").toNumber());


    QMouseEvent eRelease(QEvent::MouseMove, QPoint(42,43), QPoint(0,0),
                  Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    handler.mouseReleaseEvent(&eRelease);
    BOOST_CHECK_EQUAL(-1, engine.evaluate("buttonMove").toNumber());
    BOOST_CHECK_EQUAL(0, engine.evaluate("buttonPress").toNumber());
    BOOST_CHECK_EQUAL(2, engine.evaluate("buttonRelease").toNumber());
}

BOOST_AUTO_TEST_CASE(Modifiers) {
    auto model = std::make_shared<tinia::model::ExposedModel>();

    QString script =
            "alt = false;\n"
            "shift = false;\n"
            "ctrl = false;\n"
            "function ModifierTest(params) {}\n"
            "ModifierTest.prototype = {\n"
            "   mousePressEvent: function(event) {\n"
            "       alt = event.altKey;\n"
            "       shift = event.shiftKey;\n"
            "       ctrl = event.ctrlKey;\n"
            "   }\n"
            "}\n";

    auto& engine = tinia::qtcontroller::scripting::ScriptEngine::getInstance()->engine();

    engine.evaluate(script);

    tinia::qtcontroller::scripting::EventHandler handler("ModifierTest", "key", model, engine);

    {
        QMouseEvent ePress(QEvent::MouseMove, QPoint(42,43), QPoint(0,0),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        handler.mousePressEvent(&ePress);
        BOOST_CHECK(!engine.evaluate("alt").toBool());
        BOOST_CHECK(!engine.evaluate("shift").toBool());
        BOOST_CHECK(!engine.evaluate("ctrl").toBool());
    }

    {
        QMouseEvent ePress(QEvent::MouseMove, QPoint(42,43), QPoint(0,0),
                           Qt::LeftButton, Qt::LeftButton, Qt::AltModifier);
        handler.mousePressEvent(&ePress);
        BOOST_CHECK(engine.evaluate("alt").toBool());
        BOOST_CHECK(!engine.evaluate("shift").toBool());
        BOOST_CHECK(!engine.evaluate("ctrl").toBool());
    }

    {
        QMouseEvent ePress(QEvent::MouseMove, QPoint(42,43), QPoint(0,0),
                           Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier);
        handler.mousePressEvent(&ePress);
        BOOST_CHECK(!engine.evaluate("alt").toBool());
        BOOST_CHECK(!engine.evaluate("shift").toBool());
        BOOST_CHECK(engine.evaluate("ctrl").toBool());
    }

    {
        QMouseEvent ePress(QEvent::MouseMove, QPoint(42,43), QPoint(0,0),
                           Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
        handler.mousePressEvent(&ePress);
        BOOST_CHECK(!engine.evaluate("alt").toBool());
        BOOST_CHECK(engine.evaluate("shift").toBool());
        BOOST_CHECK(!engine.evaluate("ctrl").toBool());
    }

    {
        QMouseEvent ePress(QEvent::MouseMove, QPoint(42,43), QPoint(0,0),
                           Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier|Qt::AltModifier);
        handler.mousePressEvent(&ePress);
        BOOST_CHECK(engine.evaluate("alt").toBool());
        BOOST_CHECK(engine.evaluate("shift").toBool());
        BOOST_CHECK(!engine.evaluate("ctrl").toBool());
    }

    {
        QMouseEvent ePress(QEvent::MouseMove, QPoint(42,43), QPoint(0,0),
                           Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier | Qt::ShiftModifier|Qt::AltModifier);
        handler.mousePressEvent(&ePress);
        BOOST_CHECK(engine.evaluate("alt").toBool());
        BOOST_CHECK(engine.evaluate("shift").toBool());
        BOOST_CHECK(engine.evaluate("ctrl").toBool());
    }

    {
        QMouseEvent ePress(QEvent::MouseMove, QPoint(42,43), QPoint(0,0),
                           Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier | Qt::ShiftModifier);
        handler.mousePressEvent(&ePress);
        BOOST_CHECK(!engine.evaluate("alt").toBool());
        BOOST_CHECK(engine.evaluate("shift").toBool());
        BOOST_CHECK(engine.evaluate("ctrl").toBool());
    }

    {
        QMouseEvent ePress(QEvent::MouseMove, QPoint(42,43), QPoint(0,0),
                           Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier | Qt::AltModifier);
        handler.mousePressEvent(&ePress);
        BOOST_CHECK(engine.evaluate("alt").toBool());
        BOOST_CHECK(!engine.evaluate("shift").toBool());
        BOOST_CHECK(engine.evaluate("ctrl").toBool());
    }
}

BOOST_AUTO_TEST_CASE(KeyTest) {
    QString script =
            "var key ='notset';\n"
            "function KeyTest(params) {\n"
            "    key = params.key;\n"
            "}\n";
    QScriptEngine engine;
    auto model = std::make_shared<tinia::model::ExposedModel>();
    engine.evaluate(script);

    tinia::qtcontroller::scripting::EventHandler handler("KeyTest", "KeyFromTest", model, engine);

    BOOST_CHECK_EQUAL("KeyFromTest", engine.evaluate("key").toString().toStdString());

}


BOOST_AUTO_TEST_CASE(MapTest) {
    QString script =
            "var keyMap ='notset';\n"
            "function KeyTest(params) {\n"
            "    keyMap = params.key;\n"
            "}\n";
    QScriptEngine engine;
    auto model = std::make_shared<tinia::model::ExposedModel>();
    engine.evaluate(script);

    std::map<std::string, std::string> params;
    params["key"] = "KeyFromTest";
    tinia::qtcontroller::scripting::EventHandler handler("KeyTest", params, model, engine);

    BOOST_CHECK_EQUAL("KeyFromTest", engine.evaluate("keyMap").toString().toStdString());

}

BOOST_AUTO_TEST_SUITE_END()



