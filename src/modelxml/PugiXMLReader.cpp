#include "tinia/model/impl/xml/PugiXMLReader.hpp"
#include <pugixml.hpp>
#include <sstream>
#include <boost/property_tree/xml_parser.hpp>
#include "tinia/model/impl/TypeToXSDType.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace {

     // These are helper classes to ease the schema traversal
    struct SchemaElementHandler {
        /**
         * @brief handleSchemaNoda
         * @param node
         * @param model
         * @return true if the handler handled the data
         */
        virtual bool handleSchemaNode(const std::string& name,
                                      const std::string& type,
                                      pugi::xml_node& node,
                                      tinia::model::ExposedModel& model) const = 0;
    };

    template<class SimpleType>
    struct SimpleTypeSchemaElementHandler : public SchemaElementHandler {
        virtual bool handleSchemaNode(const std::string& name,
                                      const std::string& type,
                                      pugi::xml_node &node,
                                      tinia::model::ExposedModel &model) const
        {
            std::string expectedType = tinia::model::impl::TypeToXSDType<SimpleType>::getTypename();
            if (expectedType == type) {
                if (!handleRestrictions(name, type, node, model)) {
                    // We have a simple type without restriction
                    SimpleType value;
                    model.addElement<SimpleType>(name, value);
                    return true;
                } else {
                    return true;
                }
            }
            return false;
        }

    private:
        /**
         * @brief handleRestrictions
         * @param name
         * @param type
         * @param node
         * @param model
         * @return true if restrictions was handled, false otherwise
         */
        bool handleRestrictions(const std::string& name,
                                const std::string& type,
                                pugi::xml_node& node,
                                tinia::model::ExposedModel &model) const
        {
            // Check if we have restrictions:
            pugi::xml_node restrictionsNode = node.select_single_node("//xsd:restriction").node();

            if (restrictionsNode) {
                // See if it's max/min
                pugi::xml_node maxNode =
                        restrictionsNode.select_single_node("xsd:maxInclusive").node();
                pugi::xml_node minNode = restrictionsNode.select_single_node("xsd:minInclusive").node();

                if ( (!minNode) != (!maxNode) ) {
                    throw std::runtime_error("Found min or max present in schema, but not both, currently we do not support only one");
                }
                if (maxNode && minNode)
                {
                    // We have a max/min restriction
                    SimpleType minValue = boost::lexical_cast<SimpleType>(minNode.attribute("value").as_string());
                    SimpleType maxValue = boost::lexical_cast<SimpleType>(maxNode.attribute("value").as_string());

                    // Add element
                    model.addConstrainedElement<SimpleType>(name, minValue, minValue, maxValue);
                    return true;
                }
                else if (restrictionsNode.select_single_node("xsd:enumeration")){
                    // We have a restriction list
                    std::vector<SimpleType> restriction;
                    for (pugi::xml_node_iterator it = restrictionsNode.begin();
                         it != restrictionsNode.end(); ++it)
                    {
                        restriction.push_back(boost::lexical_cast<SimpleType>(it->attribute("value").as_string()));
                    }

                    model.addElementWithRestriction<SimpleType>(name, restriction[0], restriction);
                    return true;
                }

            }

            // Didn't find any restrictions
            return false;
        }
    };

    struct ViewerSchemaElementHandler : public SchemaElementHandler {
        bool handleSchemaNode(const std::string &name, const std::string &type, pugi::xml_node &node, tinia::model::ExposedModel &model) const {
            if (type == "xsd:complexType") {
                // Ensure that we find all properties of the viewer
                const char* namesToFind[] = {"width",
                                             "height",
                                             "projection",
                                             "modelview"};
                for (size_t i = 0; i < 4; ++i) {
                    std::string searchString = std::string("xsd:sequence/xsd:element[@name='")
                            + namesToFind[i] + "']";
                    if (!node.select_single_node(searchString.c_str()).node()) {
                        return false;
                    }
                }

                model.addElement<tinia::model::Viewer>(name, tinia::model::Viewer());
                return true;
            }
            return false;
        }
    };

    struct SchemaHandlers;
    const SchemaHandlers& getSchemaHandlersInstance();

    /**
     * A collection of handlers( this is a singleton)
     */
    struct SchemaHandlers {
        friend const SchemaHandlers& getSchemaHandlersInstance();
        const std::vector<boost::shared_ptr<SchemaElementHandler> >& getHandlers() const {
            return m_handlers;
        }

    private:
        SchemaHandlers() {
            m_handlers.push_back(boost::make_shared<ViewerSchemaElementHandler>());
            m_handlers.push_back(boost::make_shared<SimpleTypeSchemaElementHandler<bool> >());
            m_handlers.push_back(boost::make_shared<SimpleTypeSchemaElementHandler<int> >());
            m_handlers.push_back(boost::make_shared<SimpleTypeSchemaElementHandler<double> >());
            m_handlers.push_back(boost::make_shared<SimpleTypeSchemaElementHandler<std::string> >());
            m_handlers.push_back(boost::make_shared<SimpleTypeSchemaElementHandler<float> >());
        }
        std::vector<boost::shared_ptr<SchemaElementHandler> > m_handlers;
    };

    const SchemaHandlers& getSchemaHandlersInstance() {
        static SchemaHandlers handlers;
        return handlers;
    }

}
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
        // We can update everything except complex type
        if (schemaElement.getXSDType() == "xsd:complexType") {
            StringStringPTree complexInformation;
            std::stringstream xmlAsText;
            stateIt->print(xmlAsText);
            boost::property_tree::read_xml(xmlAsText, complexInformation);
            m_exposedModel->updateElementFromPTree(key, complexInformation);
        } else {
            // All other types may be updated directly
            m_exposedModel->updateElementFromString(key, stateIt->first_child().value());
        }


    }
}

void PugiXMLReader::readSchema(const std::string &documentString)
{
    pugi::xml_document document;

    // Load document into stringstream
    std::stringstream documentStream(documentString);
    pugi::xml_parse_result result = document.load(documentStream);

    if (!result) {
        throw std::runtime_error("Could not parse XML document");
    }

    // Find the base schema
    pugi::xml_node schemaNode = document.select_single_node("//xsd:schema/xsd:element[@name='State']/xsd:complexType/xsd:all").node();
    if (!schemaNode) {
        throw std::runtime_error("Could not find a xsd:schema node in the document");
    }


    // Now we loop through each element and add
    for (pugi::xml_node_iterator schemaIt = schemaNode.begin(); schemaIt != schemaNode.end();
         ++schemaIt)
    {

        std::string name = schemaIt->attribute("name").as_string();
        std::string type = "";

        if(std::string(schemaIt->name()) == std::string("xsd:complexType")) {
            type = "xsd:complexType";
        }

        else if (schemaIt->attribute("type")) {
            type = schemaIt->attribute("type").as_string();
        }
        else
        {
            pugi::xml_node restrictionNode = schemaIt->select_single_node("//xsd:restriction").node();

            if (restrictionNode) {
                type = restrictionNode.attribute("base").as_string();
            }
        }

        if (type =="") {
            throw std::runtime_error("Could not determine type of element");
        }

        // Get handlers and loop through
        const std::vector<boost::shared_ptr<SchemaElementHandler> >& handlers =
                getSchemaHandlersInstance().getHandlers();

        bool foundHandler = false;
        for (size_t i = 0; i < handlers.size(); ++i) {
            if (handlers[i]->handleSchemaNode(name, type, *schemaIt, *m_exposedModel)) {
                // We have found the correct handler
                foundHandler = true;
                break;
            }
        }

        if (!foundHandler) {
            throw std::runtime_error("Did not know how to handle element");
        }
    }

}

}}}}
