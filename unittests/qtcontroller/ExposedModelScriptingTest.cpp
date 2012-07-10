#include <boost/test/unit_test.hpp>
#include <tinia/qtcontroller/scripting/ExposedModel.hpp>
#include <QScriptEngine>

BOOST_AUTO_TEST_SUITE(ExposedModelScripting)

namespace {
struct ModelScriptingFixture {
    std::shared_ptr<tinia::model::ExposedModel> model;
    tinia::qtcontroller::scripting::ExposedModel scriptingModel;
    QScriptEngine eng;

    ModelScriptingFixture()
        : model(new tinia::model::ExposedModel()), scriptingModel(model, &eng)
    {
        ;
    }
};
}

BOOST_FIXTURE_TEST_CASE(UpdateWithoutScriptString, ModelScriptingFixture) {
    model->addElement<std::string>("key", "value");
    scriptingModel.updateElement("key", QString("newValue"));

    std::string newValue;
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL("newValue", newValue);
}


BOOST_FIXTURE_TEST_CASE(UpdateWithoutScriptInt, ModelScriptingFixture) {
    model->addElement<int>("key", 0);
    scriptingModel.updateElement("key", 1);

    int newValue;
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL(1, newValue);

    // Also test string conversion:
    scriptingModel.updateElement("key", QString("2"));
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL(2, newValue);
}

BOOST_FIXTURE_TEST_CASE(UpdateWithoutScriptDouble, ModelScriptingFixture) {
    model->addElement<double>("key", 0.0);
    scriptingModel.updateElement("key", 1.0);

    double newValue;
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL(1, newValue);

    // Also test string conversion:

    scriptingModel.updateElement("key", QString("2"));
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL(2, newValue);
}

BOOST_FIXTURE_TEST_CASE(UpdateWithoutScriptBool, ModelScriptingFixture) {
    model->addElement<bool>("key", false);
    scriptingModel.updateElement("key", true);

    bool newValue;
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL(true, newValue);
}

BOOST_FIXTURE_TEST_CASE(UpdateWithScriptString, ModelScriptingFixture) {

    model->addElement<std::string>("key", "value");
    auto scriptObject = eng.newQObject(&scriptingModel);
    auto function = eng.evaluate("(function(exposedModel) {exposedModel.updateElement('key', 'newValue'); })");
    function.call(QScriptValue(), QScriptValueList() << scriptObject);

    std::string newValue;
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL("newValue", newValue);
}

BOOST_FIXTURE_TEST_CASE(UpdateWithScriptInt, ModelScriptingFixture) {

    model->addElement<int>("key", 0);
    auto scriptObject = eng.newQObject(&scriptingModel);

    auto function = eng.evaluate("(function(exposedModel) {exposedModel.updateElement('key', 1); })");
    function.call(QScriptValue(), QScriptValueList() << scriptObject);

    int newValue;
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL(1, newValue);

    // Check that strings work here as well;
    auto functionString = eng.evaluate("(function(exposedModel) {exposedModel.updateElement('key', '2'); })");
    functionString.call(QScriptValue(), QScriptValueList() << scriptObject);
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL(2, newValue);

}

BOOST_FIXTURE_TEST_CASE(UpdateWithScriptDouble, ModelScriptingFixture) {

    model->addElement<double>("key", 0.0);
    auto scriptObject = eng.newQObject(&scriptingModel);

    auto function = eng.evaluate("(function(exposedModel) {exposedModel.updateElement('key', 1.0); })");
    function.call(QScriptValue(), QScriptValueList() << scriptObject);

    double newValue;
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL(1, newValue);

    // Check that strings work here as well;
    auto functionString = eng.evaluate("(function(exposedModel) {exposedModel.updateElement('key', '2.0'); })");
    functionString.call(QScriptValue(), QScriptValueList() << scriptObject);
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL(2, newValue);
}

BOOST_FIXTURE_TEST_CASE(UpdateWithScriptBool, ModelScriptingFixture) {

    model->addElement<bool>("key", true);
    auto scriptObject = eng.newQObject(&scriptingModel);

    auto function = eng.evaluate("(function(exposedModel) {exposedModel.updateElement('key', false); })");
    function.call(QScriptValue(), QScriptValueList() << scriptObject);

    bool newValue;
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL(false, newValue);

    // Check that strings work here as well;
    auto functionString = eng.evaluate("(function(exposedModel) {exposedModel.updateElement('key', '1'); })");
    functionString.call(QScriptValue(), QScriptValueList() << scriptObject);
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL(true, newValue);
}

BOOST_FIXTURE_TEST_CASE(GetElementValueString, ModelScriptingFixture) {
    model->addElement<std::string>("key", "value");


    auto scriptObject = eng.newQObject(&scriptingModel);
    auto function = eng.evaluate("(function(model) { return model.getElementValue('key'); })");
    auto result = function.call(QScriptValue(), QScriptValueList() << scriptObject);
    BOOST_CHECK_EQUAL("value", result.toString().toStdString());
}

BOOST_FIXTURE_TEST_CASE(GetElementValueInt, ModelScriptingFixture) {
    model->addElement<int>("key", 42);


    auto scriptObject = eng.newQObject(&scriptingModel);
    auto function = eng.evaluate("(function(model) { return model.getElementValue('key'); })");
    auto result = function.call(QScriptValue(), QScriptValueList() << scriptObject);
    BOOST_CHECK_EQUAL(42, result.toNumber());

    // Also ensure the type is correct:
    auto functionType = eng.evaluate("(function(model) { return typeof(model.getElementValue('key')); })");
    auto typeResult = functionType.call(QScriptValue(), QScriptValueList() << scriptObject);
    BOOST_CHECK_EQUAL("number", typeResult.toString().toStdString());
}

BOOST_FIXTURE_TEST_CASE(GetElementValueDouble, ModelScriptingFixture) {
    model->addElement<double>("key", 42.5);


    auto scriptObject = eng.newQObject(&scriptingModel);
    auto function = eng.evaluate("(function(model) { return model.getElementValue('key'); })");
    auto result = function.call(QScriptValue(), QScriptValueList() << scriptObject);
    BOOST_CHECK_EQUAL(42.5, result.toNumber());

    // Also ensure the type is correct:
    auto functionType = eng.evaluate("(function(model) { return typeof(model.getElementValue('key')); })");
    auto typeResult = functionType.call(QScriptValue(), QScriptValueList() << scriptObject);
    BOOST_CHECK_EQUAL("number", typeResult.toString().toStdString());
}

BOOST_FIXTURE_TEST_CASE(GetElementValueBool, ModelScriptingFixture) {
    model->addElement<bool>("key", true);


    auto scriptObject = eng.newQObject(&scriptingModel);
    auto function = eng.evaluate("(function(model) { return model.getElementValue('key'); })");
    auto result = function.call(QScriptValue(), QScriptValueList() << scriptObject);
    BOOST_CHECK_EQUAL(true, result.toBool());

    // Also ensure the type is correct:
    auto functionType = eng.evaluate("(function(model) { return typeof(model.getElementValue('key')); })");
    auto typeResult = functionType.call(QScriptValue(), QScriptValueList() << scriptObject);
    BOOST_CHECK_EQUAL("boolean", typeResult.toString().toStdString());
}

BOOST_FIXTURE_TEST_CASE(ViewerTestWithoutModelWithoutScript, ModelScriptingFixture) {
    tinia::qtcontroller::scripting::Viewer v(&eng);
    QString modelView("Array(");
    for(int i = 0; i < 16; ++i) {
        modelView += QString::number(i);
        if( i  < 15 ) {
            modelView +=", ";
        }
    }
    modelView += ")";

    v.updateElement("modelview", eng.evaluate(modelView));

    QString projection("Array(");
    for(int i = 0; i < 16; ++i) {
        projection += QString::number(15 - i);
        if( i  < 15 ) {
            projection +=", ";
        }
    }
    projection += ")";

    v.updateElement("projection", eng.evaluate(projection));

    v.updateElement("height", QScriptValue(100));
    v.updateElement("width", QScriptValue(42));

    tinia::model::Viewer modelViewer = v.viewer();

    for(int i = 0; i < 16; ++i) {
        BOOST_CHECK_EQUAL(modelViewer. modelviewMatrix[i], i);
        BOOST_CHECK_EQUAL(modelViewer. projectionMatrix[i], 15-i);
    }
    BOOST_CHECK_EQUAL(modelViewer.height, 100);
    BOOST_CHECK_EQUAL(modelViewer.width, 42);
}

BOOST_FIXTURE_TEST_CASE(ViewerTestWithoutModelWithScript, ModelScriptingFixture) {
    tinia::qtcontroller::scripting::Viewer v(&eng);

    QString script = "(function(viewer) { "
            "viewer.updateElement('modelview', "
            "    Array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)); "
            "viewer.updateElement('projection', "
            "    Array(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)); "
            "viewer.updateElement('height', 100);"
            "viewer.updateElement('width', 42);"
            "})";

    auto function = eng.evaluate(script);
    function.call(QScriptValue(), QScriptValueList() << eng.newQObject(&v));

    tinia::model::Viewer modelViewer = v.viewer();

    for(int i = 0; i < 16; ++i) {
        BOOST_CHECK_EQUAL(modelViewer. modelviewMatrix[i], i + 1);
        BOOST_CHECK_EQUAL(modelViewer. projectionMatrix[i], 16-i);
    }
    BOOST_CHECK_EQUAL(modelViewer.height, 100);
    BOOST_CHECK_EQUAL(modelViewer.width, 42);
    for(int i = 0; i < 16; ++i) {
        QString fetchModelView = "(function(viewer) { return viewer.getElementValue('modelview')[" + QString::number(i) + "]; })";
        auto fetchModelViewFunction = eng.evaluate(fetchModelView);
        auto modelViewResult = fetchModelViewFunction.call(QScriptValue(), QScriptValueList() << eng.newQObject(&v));
        BOOST_CHECK_EQUAL(i + 1, modelViewResult.toNumber());


        QString fetchProjection = "(function(viewer) { return viewer.getElementValue('projection')[" + QString::number(i)+ "]; })";
        auto fetchProjectionFunction = eng.evaluate(fetchProjection);
        auto projectionResult = fetchProjectionFunction.call(QScriptValue(), QScriptValueList() << eng.newQObject(&v));
        BOOST_CHECK_EQUAL(16 - i, projectionResult.toNumber());
    }

    QString fetchHeight = "(function(viewer) { return viewer.getElementValue('height'); })";
    BOOST_CHECK_EQUAL(100, eng.evaluate(fetchHeight).call(QScriptValue(), QScriptValueList() << eng.newQObject(&v)).toNumber());

    QString fetchWidth = "(function(viewer) { return viewer.getElementValue('width'); })";
    BOOST_CHECK_EQUAL(42, eng.evaluate(fetchWidth).call(QScriptValue(), QScriptValueList() << eng.newQObject(&v)).toNumber());
}



BOOST_FIXTURE_TEST_CASE(ViewerTestWithModel, ModelScriptingFixture)  {

    tinia::model::Viewer v;
    v.height = 100;
    v.width = 42;
    for(int i = 0; i < 16; ++i) {
        v.modelviewMatrix[i] = i + 1;
        v.projectionMatrix[i] = 16 - i;
    }
    model->addElement("viewer", v);
    for(int i = 0; i < 16; ++i) {
        QString fetchModelView = "(function(model) { return model.getElementValue('viewer').getElementValue('modelview')[" + QString::number(i) + "]; })";
        auto fetchModelViewFunction = eng.evaluate(fetchModelView);
        auto modelViewResult = fetchModelViewFunction.call(QScriptValue(), QScriptValueList() << eng.newQObject(&scriptingModel));
        BOOST_CHECK_EQUAL(i + 1, modelViewResult.toNumber());


        QString fetchProjection = "(function(model) {  return model.getElementValue('viewer').getElementValue('projection')[" + QString::number(i)+ "]; })";
        auto fetchProjectionFunction = eng.evaluate(fetchProjection);
        auto projectionResult = fetchProjectionFunction.call(QScriptValue(), QScriptValueList() << eng.newQObject(&scriptingModel));
        BOOST_CHECK_EQUAL(16 - i, projectionResult.toNumber());
    }

    QString fetchHeight = "(function(model) { return model.getElementValue('viewer').getElementValue('height'); })";
    BOOST_CHECK_EQUAL(100, eng.evaluate(fetchHeight).call(QScriptValue(), QScriptValueList() << eng.newQObject(&scriptingModel)).toNumber());

    QString fetchWidth = "(function(model) { return model.getElementValue('viewer').getElementValue('width'); })";
    BOOST_CHECK_EQUAL(42, eng.evaluate(fetchWidth).call(QScriptValue(), QScriptValueList() << eng.newQObject(&scriptingModel)).toNumber());


    // Now we update from javascript
    QString script = "(function(model) { "
            "var viewer = model.getElementValue('viewer');"
            "viewer.updateElement('projection', "
            "    Array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)); "
            "viewer.updateElement('modelview', "
            "    Array(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)); "
            "viewer.updateElement('width', 100);"
            "viewer.updateElement('height', 42);"

            "model.updateElement('viewer', viewer);"

            "})";

    auto function = eng.evaluate(script);
    function.call(QScriptValue(), QScriptValueList() << eng.newQObject(&scriptingModel));


    tinia::model::Viewer modelViewer;
    model->getElementValue("viewer", modelViewer);

    for(int i = 0; i < 16; ++i) {
        BOOST_CHECK_EQUAL(modelViewer. modelviewMatrix[i], 16 - i );
        BOOST_CHECK_EQUAL(modelViewer. projectionMatrix[i], i + 1);
    }
}


BOOST_FIXTURE_TEST_CASE(ListenerTestCase, ModelScriptingFixture) {
    QString script =
            "var listenerCalled = false;\n"
            "var valueWas = -1;\n"
            "function main(model) {\n"
            "   model.addLocalListener('key', function(key, value) {\n"
            "       listenerCalled = true;\n"
            "       valueWas = value;\n"
            "   });"
            "}";

    eng.evaluate(script);

    model->addElement("key", 41);
    eng.evaluate("main").call(QScriptValue(), QScriptValueList() << eng.newQObject(&scriptingModel));
    BOOST_CHECK(!eng.evaluate("listenerCalled").toBool());
    BOOST_CHECK_EQUAL(-1, eng.evaluate("valueWas").toNumber());

    model->updateElement("key", 42);

    BOOST_CHECK(eng.evaluate("listenerCalled").toBool());
    BOOST_CHECK_EQUAL(42, eng.evaluate("valueWas").toNumber());

}



BOOST_AUTO_TEST_SUITE_END()
