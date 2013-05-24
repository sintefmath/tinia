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

BOOST_FIXTURE_TEST_CASE(UpdateViewer, PugiXMLReaderTestFixture) {
    model->addElement("viewer", tinia::model::Viewer());

    // Just making sure we actually update.
    BOOST_CHECK_NE(10, model->getElementValue<tinia::model::Viewer>("viewer").width);
    BOOST_CHECK_NE(30, model->getElementValue<tinia::model::Viewer>("viewer").projectionMatrix[0]);
    std::string documentWithViewer = "<?xml version=\"1.0\"?>"
                                   "<ExposedModelUpdate xmlns=\"http://cloudviz.sintef.no/V1/model\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://cloudviz.sintef.no/V1/model/ExposedModelUpdateSchema.xsd\" revision=\"10456\">"
                                   "    <StateSchema xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
                                   "        <xsd:schema>"
                                   "            <xsd:element name=\"State\">"
                                   "                 <xsd:complexType>"
                                   "                      <xsd:all>"
            "<xsd:complexType name=\"viewer\">"
                "<xsd:sequence>"
                    "<xsd:element name=\"width\" type=\"xsd:integer\"/>"
                    "<xsd:element name=\"height\" type=\"xsd:integer\"/>"
                    "<xsd:element name=\"projection\">"
                        "<xsd:restriction>"
                            "<xsd:simpleType>"
                                "<xsd:list itemType=\"xsd:float\"/>"
                            "</xsd:simpleType>"
                            "<xsd:length value=\"16\"/>"
                        "</xsd:restriction>"
                    "</xsd:element>"
                    "<xsd:element name=\"modelview\">"
                        "<xsd:restriction>"
                            "<xsd:simpleType>"
                                "<xsd:list itemType=\"xsd:float\"/>"
                            "</xsd:simpleType>"
                            "<xsd:length value=\"16\"/>"
                        "</xsd:restriction>"
                    "</xsd:element>"
                    "<xsd:element name=\"timestamp\" type=\"xsd:double\"/>"
                    "<xsd:element name=\"sceneView\" type=\"xsd:string\"/>"
                "</xsd:sequence>"
            "</xsd:complexType>"
                                   "                      </xsd:all>"
                                   "                  </xsd:complexType>"
                                   "            </xsd:element>"
                                   "        </xsd:schema>"
                                   "   </StateSchema>"
                                   "   <State>"
                                    "<viewer>"
                                              "                          <width>10</width>"
                                              "                          <height>782</height>"
                                              "                          <projection>30 0 0 0 0 1 0 0 0 0 -1.0202020406723022 -1 0 0 -18.5638427734375 0</projection>"
                                              "<modelview>0.7354713678359985 -0.6516221761703491 0.18566226959228516 0 0.2348913997411728 0.502234697341919 0.832217812538147 0 -0.6355375647544861 -0.5684619545936584 0.5224396586418152 0 -65.47185516357422 -58.561859130859375 -436.5816650390625 1</modelview>"
                                              "<timestamp>20</timestamp>"
                                           "<sceneView>---oooOOOooo---</sceneView>"
                       "</viewer>"
                                   "   </State>"
                                   "   <GuiLayout>"
                                   "   </GuiLayout>"
                                   "</ExposedModelUpdate>";

    reader.readState(documentWithViewer);
    BOOST_CHECK_EQUAL(10, model->getElementValue<tinia::model::Viewer>("viewer").width);
    BOOST_CHECK_EQUAL(30, model->getElementValue<tinia::model::Viewer>("viewer").projectionMatrix[0]);
}

BOOST_FIXTURE_TEST_CASE(SchemaReadAssertNoThrow, PugiXMLReaderTestFixture) {
    std::string correctDocument = "<?xml version=\"1.0\"?>"
                                   "<ExposedModelUpdate xmlns=\"http://cloudviz.sintef.no/V1/model\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://cloudviz.sintef.no/V1/model/ExposedModelUpdateSchema.xsd\" revision=\"10456\">"
                                   "    <StateSchema xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
                                   "        <xsd:schema>"
                                   "            <xsd:element name=\"State\">"
                                   "                 <xsd:complexType>"
                                   "                      <xsd:all>"
            "<xsd:complexType name=\"viewer\">"
                "<xsd:sequence>"
                    "<xsd:element name=\"width\" type=\"xsd:integer\"/>"
                    "<xsd:element name=\"height\" type=\"xsd:integer\"/>"
                    "<xsd:element name=\"projection\">"
                        "<xsd:restriction>"
                            "<xsd:simpleType>"
                                "<xsd:list itemType=\"xsd:float\"/>"
                            "</xsd:simpleType>"
                            "<xsd:length value=\"16\"/>"
                        "</xsd:restriction>"
                    "</xsd:element>"
                    "<xsd:element name=\"modelview\">"
                        "<xsd:restriction>"
                            "<xsd:simpleType>"
                                "<xsd:list itemType=\"xsd:float\"/>"
                            "</xsd:simpleType>"
                            "<xsd:length value=\"16\"/>"
                        "</xsd:restriction>"
                    "</xsd:element>"
                    "<xsd:element name=\"timestamp\" type=\"xsd:double\"/>"
                    "<xsd:element name=\"sceneView\" type=\"xsd:string\"/>"
                "</xsd:sequence>"
            "</xsd:complexType>"
                                   "                      </xsd:all>"
                                   "                  </xsd:complexType>"
                                   "            </xsd:element>"
                                   "        </xsd:schema>"
                                   "   </StateSchema>"
                                   "   <State>"
                                    "<viewer>"
                                              "                          <width>10</width>"
                                              "                          <height>782</height>"
                                              "                          <projection>30 0 0 0 0 1 0 0 0 0 -1.0202020406723022 -1 0 0 -18.5638427734375 0</projection>"
                                              "<modelview>0.7354713678359985 -0.6516221761703491 0.18566226959228516 0 0.2348913997411728 0.502234697341919 0.832217812538147 0 -0.6355375647544861 -0.5684619545936584 0.5224396586418152 0 -65.47185516357422 -58.561859130859375 -436.5816650390625 1</modelview>"
                                              "<timestamp>20</timestamp>"
                                           "<sceneView>---oooOOOooo---</sceneView>"
                       "</viewer>"
                                   "   </State>"
                                   "   <GuiLayout>"
                                   "   </GuiLayout>"
                                   "</ExposedModelUpdate>";

    BOOST_CHECK_NO_THROW(reader.readSchema(correctDocument));
}

BOOST_FIXTURE_TEST_CASE(SchemaReadAssertThrowOnWithoutSchema, PugiXMLReaderTestFixture) {
    std::string incorrectDocument = "<?xml version=\"1.0\"?>"
                                   "<ExposedModelUpdate xmlns=\"http://cloudviz.sintef.no/V1/model\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://cloudviz.sintef.no/V1/model/ExposedModelUpdateSchema.xsd\" revision=\"10456\">"
                                   "    <StateSchema xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
                                   "   </StateSchema>"
                                   "   <State>"
                                    "<viewer>"
                                              "                          <width>10</width>"
                                              "                          <height>782</height>"
                                              "                          <projection>30 0 0 0 0 1 0 0 0 0 -1.0202020406723022 -1 0 0 -18.5638427734375 0</projection>"
                                              "<modelview>0.7354713678359985 -0.6516221761703491 0.18566226959228516 0 0.2348913997411728 0.502234697341919 0.832217812538147 0 -0.6355375647544861 -0.5684619545936584 0.5224396586418152 0 -65.47185516357422 -58.561859130859375 -436.5816650390625 1</modelview>"
                                              "<timestamp>20</timestamp>"
                                           "<sceneView>---oooOOOooo---</sceneView>"
                       "</viewer>"
                                   "   </State>"
                                   "   <GuiLayout>"
                                   "   </GuiLayout>"
                                   "</ExposedModelUpdate>";

    BOOST_CHECK_THROW(reader.readSchema(incorrectDocument), std::exception);
}

BOOST_FIXTURE_TEST_CASE(SchemaReadAssertThrowOnWithFaultyDocument, PugiXMLReaderTestFixture) {
    std::string incorrectDocument = "<?xml version=\"1.0\"?>"
                                   "   </State>"
                                   "   <GuiLayout>"
                                   "   </GuiLayout>"
                                   "</ExposedModelUpdate>";

    BOOST_CHECK_THROW(reader.readSchema(incorrectDocument), std::exception);
}

BOOST_FIXTURE_TEST_CASE(SchemaReadCheckSimpleDocument, PugiXMLReaderTestFixture) {
    std::string correctDocument = "<?xml version=\"1.0\"?>"
                                   "<ExposedModelUpdate xmlns=\"http://cloudviz.sintef.no/V1/model\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://cloudviz.sintef.no/V1/model/ExposedModelUpdateSchema.xsd\" revision=\"10456\">"
                                   "    <StateSchema xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
                                   "        <xsd:schema>"
                                   "            <xsd:element name=\"State\">"
                                   "                 <xsd:complexType>"
                                   "                      <xsd:all>"
                                   "                          <xsd:element name=\"stringKey\" type=\"xsd:string\" />"
                                   "                           <xsd:element name=\"intKey\" type=\"xsd:integer\" />"
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

    reader.readSchema(correctDocument);
    BOOST_CHECK_NO_THROW(reader.readState(correctDocument));

    BOOST_CHECK_EQUAL("new value", model->getElementValue<std::string>("stringKey"));
    BOOST_CHECK_EQUAL(4, model->getElementValue<int>("intKey"));
}

BOOST_AUTO_TEST_SUITE_END()
