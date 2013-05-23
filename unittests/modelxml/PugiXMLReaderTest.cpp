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

BOOST_AUTO_TEST_SUITE_END()
