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

    // Find State
    pugi::xml_node stateNode = document.select_single_node("*/State").node();
    if (!stateNode) {
        throw std::runtime_error("Could not find a State node in the document");
    }

    // Now we traverse
    for (pugi::xml_node_iterator stateIt = stateNode.begin(); stateIt != stateNode.end();
         ++stateIt)
    {
        // We need to get the name:
        std::string key = stateIt->name();

        tinia::model::StateSchemaElement schemaElement
                = m_exposedModel->getStateSchemaElement(key);

        // We can update everything except complex type and matrix easily
        if (schemaElement.getXSDType() == "xsd:complexType") {
            // We have a complex type
        }
        else if (schemaElement.getLength() == ElementData::MATRIX_LENGTH) {
            // Matrix type
        } else {
            // All other types may be updated directly
            m_exposedModel->updateElementFromString(key, stateIt->first_child().value());
        }


    }
}

}}}}
