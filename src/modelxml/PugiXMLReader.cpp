#include "tinia/model/impl/xml/PugiXMLReader.hpp"
#include <pugixml.hpp>
#include <sstream>

namespace tinia { namespace model { namespace impl { namespace xml {

PugiXMLReader::PugiXMLReader(boost::shared_ptr<tinia::model::ExposedModel> exposedModel)
    : m_exposedModel(exposedModel)
{
}

void PugiXMLReader::readState(const std::string& documentString) {
    pugi::xml_document document;

    // Load document into stringstream
    std::stringstream documentStream(documentString);
    pugi::xml_parse_result result = document.load(documentStream);

    if (!result) {
        throw std::runtime_error("Could not parse XML document");
    }
}

}}}}
