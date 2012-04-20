#pragma once
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <vector>

#include <libxml/tree.h>
#include <libxml/xmlreader.h>

#include "tinia/policylib/PolicyLib.hpp"
#include "tinia/policylib/ElementData.hpp"
#include "tinia/policylibxml/utils.hpp"
#include "tinia/policylibxml/ElementHandler.hpp"
#include <boost/property_tree/ptree.hpp>



namespace tinia {
namespace policylibxml
{


    /** \class XMLReader
     XMLReader is responsible for parsing an xml-document from a policy.
    */

    class XMLReader {

    public:
        /** Create an XMLReader instance.
        */
        XMLReader();

        /** For a given libxml2 document describing the policy, set up the state.
            \param doc The XML document.
            \param policyLib The PolicyLib object in which to update elements per specification in the XML document.
            \return A list of the keys in the state that are updated is returned.
        */
        std::vector<std::string> parseDocument(const xmlDocPtr doc,
                                               ElementHandler &elementHandler);

    private:
        /** Helper function for parseDocument(...). The reader is advanced until the specified start element is found.
            \param reader The xmlTextReader pointing into the document.
            \param name The name of the start element to scan for.
            \return A value of -1 is returned if the start element was not found, otherwise the depth of the node is returned.
        */
        int gobbleUntilStartElement(xmlTextReaderPtr &reader, const std::string &name);

        /** Helper function for parseDocument(...). The reader is advanced until a text element is found, or the end element
            of the specified "section" is found. If a text element was found, true is returned, and the name and value of that
            element is put into the corresponding parameters.
            \param reader The xmlTextReader pointing into the document.
            \param section_name The name of the start element which the reader has passed, and of the end element it will stop at.
            \param name The name of the text element if one is found, otherwise the parameter is not changed.
            \param value The value of the text element if one is found, otherwise the parameter is not changed.
            \param state_end_found Set to true when the State-section of the policy has been read.
            \param depth the depth of the named node read.
            \return True if the named node is the head of a complex-type.
        */
        bool nextTypeContainsChildren(xmlTextReaderPtr &reader, const std::string &section_name, std::string &name, std::string &value,
                                      bool &state_end_found, int &depth);


    };




}
}










































