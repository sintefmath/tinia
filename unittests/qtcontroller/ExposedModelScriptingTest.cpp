#include <boost/test/unit_test.hpp>
#include <tinia/qtcontroller/scripting/ExposedModel.hpp>
#include <QScriptEngine>

BOOST_AUTO_TEST_SUITE(ExposedModelScripting)

namespace {
struct ModelScriptingFixture {
    std::shared_ptr<tinia::model::ExposedModel> model;
    tinia::qtcontroller::scripting::ExposedModel scriptingModel;
    ModelScriptingFixture()
        : model(new tinia::model::ExposedModel()), scriptingModel(model)
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

    // Also test string conversion:
    scriptingModel.updateElement("key", QString("0"));
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL(false, newValue);
}

BOOST_FIXTURE_TEST_CASE(UpdateWithScriptString, ModelScriptingFixture) {
    QScriptEngine eng;
    model->addElement<std::string>("key", "value");
    auto scriptObject = eng.newQObject(&scriptingModel);
    auto function = eng.evaluate("(function(exposedModel) {exposedModel.updateElement('key', 'newValue'); })");
    function.call(QScriptValue(), QScriptValueList() << scriptObject);

    std::string newValue;
    model->getElementValue("key", newValue);
    BOOST_CHECK_EQUAL("newValue", newValue);
}

BOOST_FIXTURE_TEST_CASE(UpdateWithScriptInt, ModelScriptingFixture) {
    QScriptEngine eng;
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
    QScriptEngine eng;
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
    QScriptEngine eng;
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



BOOST_AUTO_TEST_SUITE_END()
