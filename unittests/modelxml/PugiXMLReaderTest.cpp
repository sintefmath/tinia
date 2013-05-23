#include <boost/test/unit_test.hpp>
#include "tinia/model.hpp"
#include "tinia/model/impl/xml/PugiXMLReader.hpp"

BOOST_AUTO_TEST_SUITE(PugiXMLReaderTest)

BOOST_AUTO_TEST_CASE(AssertCorrectConstruction) {
    boost::shared_ptr<tinia::model::ExposedModel>
            model(new tinia::model::ExposedModel);

    BOOST_CHECK_NO_THROW(tinia::model::impl::xml::PugiXMLReader reader(model));
}

struct PugiXMLReaderTestFixture {
    boost::shared_ptr<tinia::model::ExposedModel> model;
    tinia::model::impl::xml::PugiXMLReader reader;

    PugiXMLReaderTestFixture()
        : model(new tinia::model::ExposedModel),
        reader(model) {}
};

BOOST_FIXTURE_TEST_CASE(ReadFaultyDocumentAssertThrow, PugiXMLReaderTestFixture) {
    std::string faultyDocument = "<?xml><start>";

    BOOST_CHECK_THROW(reader.readState(faultyDocument), std::exception);

}

BOOST_FIXTURE_TEST_CASE(ReadDocumentWithoutStateAssertThrow, PugiXMLReaderTestFixture) {
    std::string incorrectDocument = "<?xml version=\"1.0\"?>"
            "<someothertag />";

    BOOST_CHECK_THROW(reader.readState(incorrectDocument), std::exception);
}

BOOST_FIXTURE_TEST_CASE(ReadCorrectDocumentAssertNoThrow, PugiXMLReaderTestFixture) {
    // First we need to add a variable to the exposedmodel to make this correct
    model->addElement("key", 0);

    std::string correctDocument = "<?xml version=\"1.0\"?>"
                                   "<ExposedModelUpdate xmlns=\"http://cloudviz.sintef.no/V1/model\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://cloudviz.sintef.no/V1/model/ExposedModelUpdateSchema.xsd\" revision=\"10456\">"
                                   "    <StateSchema xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
                                   "        <xsd:schema>"
                                   "            <xsd:element name=\"State\">"
                                   "                 <xsd:complexType>"
                                   "                      <xsd:all>"
                                   "                          <xsd:element name=\"key\" type=\"xsd:integer\" />"
                                   "                      </xsd:all>"
                                   "                  </xsd:complexType>"
                                   "            </xsd:element>"
                                   "        </xsd:schema>"
                                   "   </StateSchema>"
                                   "   <State>"
                                   "       <key>0</key>"
                                   "   </State>"
                                   "   <GuiLayout>"
                                   "   </GuiLayout>"
                                   "</ExposedModelUpdate>";

    BOOST_CHECK_NO_THROW(reader.readState(correctDocument));
}

BOOST_FIXTURE_TEST_CASE(UpdateInt, PugiXMLReaderTestFixture) {
    // First we need to add a variable to the exposedmodel to make this correct
    model->addElement("key", 0);

    std::string correctDocument = "<?xml version=\"1.0\"?>"
                                   "<ExposedModelUpdate xmlns=\"http://cloudviz.sintef.no/V1/model\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://cloudviz.sintef.no/V1/model/ExposedModelUpdateSchema.xsd\" revision=\"10456\">"
                                   "    <StateSchema xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
                                   "        <xsd:schema>"
                                   "            <xsd:element name=\"State\">"
                                   "                 <xsd:complexType>"
                                   "                      <xsd:all>"
                                   "                          <xsd:element name=\"key\" type=\"xsd:integer\" />"
                                   "                      </xsd:all>"
                                   "                  </xsd:complexType>"
                                   "            </xsd:element>"
                                   "        </xsd:schema>"
                                   "   </StateSchema>"
                                   "   <State>"
                                   "       <key>1</key>"
                                   "   </State>"
                                   "   <GuiLayout>"
                                   "   </GuiLayout>"
                                   "</ExposedModelUpdate>";

    BOOST_CHECK_NO_THROW(reader.readState(correctDocument));
    BOOST_CHECK_EQUAL(1, model->getElementValue<int>("key"));
}

BOOST_FIXTURE_TEST_CASE(UpdateString, PugiXMLReaderTestFixture) {
    // First we need to add a variable to the exposedmodel to make this correct
    model->addElement("key", std::string("value"));

    std::string correctDocument = "<?xml version=\"1.0\"?>"
                                   "<ExposedModelUpdate xmlns=\"http://cloudviz.sintef.no/V1/model\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://cloudviz.sintef.no/V1/model/ExposedModelUpdateSchema.xsd\" revision=\"10456\">"
                                   "    <StateSchema xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
                                   "        <xsd:schema>"
                                   "            <xsd:element name=\"State\">"
                                   "                 <xsd:complexType>"
                                   "                      <xsd:all>"
                                   "                          <xsd:element name=\"key\" type=\"xsd:string\" />"
                                   "                      </xsd:all>"
                                   "                  </xsd:complexType>"
                                   "            </xsd:element>"
                                   "        </xsd:schema>"
                                   "   </StateSchema>"
                                   "   <State>"
                                   "       <key>new value</key>"
                                   "   </State>"
                                   "   <GuiLayout>"
                                   "   </GuiLayout>"
                                   "</ExposedModelUpdate>";

    BOOST_CHECK_NO_THROW(reader.readState(correctDocument));
    BOOST_CHECK_EQUAL(std::string("new value"), model->getElementValue<std::string>("key"));
}

BOOST_FIXTURE_TEST_CASE(UpdateStringAndInt, PugiXMLReaderTestFixture) {
    // First we need to add a variable to the exposedmodel to make this correct
    model->addElement("stringKey", std::string("value"));
    model->addElement("intKey", 1);
    std::string correctDocument = "<?xml version=\"1.0\"?>"
                                   "<ExposedModelUpdate xmlns=\"http://cloudviz.sintef.no/V1/model\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://cloudviz.sintef.no/V1/model/ExposedModelUpdateSchema.xsd\" revision=\"10456\">"
                                   "    <StateSchema xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
                                   "        <xsd:schema>"
                                   "            <xsd:element name=\"State\">"
                                   "                 <xsd:complexType>"
                                   "                      <xsd:all>"
                                   "                          <xsd:element name=\"stringKey\" type=\"xsd:string\" />"
                                   "                           <xsd:element name=\"intKey\" type=\"int\" />"
                                   "                      </xsd:all>"
                                   "                  </xsd:complexType>"
                                   "            </xsd:element>"
                                   "        </xsd:schema>"
                                   "   </StateSchema>"
                                   "   <State>"
                                   "       <stringKey>new value</stringKey>"
                                   "       <intKey>4</intKey>"
                                   "   </State>"
                                   "   <GuiLayout>"
                                   "   </GuiLayout>"
                                   "</ExposedModelUpdate>";

    BOOST_CHECK_NO_THROW(reader.readState(correctDocument));
    BOOST_CHECK_EQUAL(std::string("new value"),
                      model->getElementValue<std::string>("stringKey"));
    BOOST_CHECK_EQUAL(4,
                      model->getElementValue<int>("intKey"));
}



BOOST_AUTO_TEST_SUITE_END()
