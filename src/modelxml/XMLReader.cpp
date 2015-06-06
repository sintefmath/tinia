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

#include "tinia/model/impl/xml/XMLReader.hpp"
//#include "tinia/model/ExposedModel.hpp"
#include "tinia/model/impl/xml/utils.hpp"
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>

#define XMLDEBUG std::cout<< __FILE__<<__LINE__ << std::endl;



using namespace std;
namespace tinia {
namespace model {
namespace impl {
namespace xml {


XMLReader::XMLReader()
    {
    }




    // Return value: -1 if "state end found", otherwise depth of node
    int XMLReader::gobbleUntilStartElement(xmlTextReaderPtr &reader, const string &name)
    {
        bool state_begin_found = false;
        bool state_end_found   = false;
        int depth              = -1;

        int r = xmlTextReaderRead(reader);
        while ( (r==1) && (!state_begin_found) && (!state_end_found) ) {
            const int type = xmlTextReaderNodeType( reader );
            depth = xmlTextReaderDepth(reader);
            xmlChar *localName = xmlTextReaderLocalName(reader);
            if (type==1) {
                state_begin_found = (xmlStrEqual(localName, BAD_CAST name.c_str()) != 0);
                if (state_begin_found)
                    state_end_found = (xmlTextReaderIsEmptyElement(reader) != 0);
            } else
                if (type==15) {
                    state_end_found = (xmlStrEqual(localName, BAD_CAST name.c_str()) != 0);
                }
            xmlFree(localName);
            r=xmlTextReaderRead(reader);
        }

        if ( r==0)
            state_end_found = true; // It makes no sense to have gotten to the beginning with r==0 at the same time

        return state_begin_found && (!state_end_found) ? depth : -1;
    }




    // Would be natural to extract subtree f.ex. with xpath and send it to updateElement. However, if we are to keep
    // elementData free of xml, it is hard to imagine how walking the subtree can be avoided, so we might as well do it
    // this way (i.e. without xpath)

    // Is the result of 'int xmlTextReaderNodeType()' (http://xmlsoft.org/html/libxml-xmlreader.html#xmlTextReaderNodeType
    // doesn't seem to indicate this) an enum of type xmlElementType?!
    // (http://xmlsoft.org/html/libxml-tree.html#xmlElementType)

    // Return value: true if next node has a sub-tree

    // 150605: A bug was exposed when two complex types were following each other. It seems that a 'true' return value here is
    //         interpreted as the start of a complex type without considering that it also marks the end of another one.
    //         Trying to fix this.

    bool XMLReader::nextTypeContainsChildren(xmlTextReaderPtr &reader, const string &section_name, string &name, string &value,
                                     bool &state_end_found, int &depth)
    {
        state_end_found = false;
        depth = -1;
        int r = 1;
        bool done=false, subtree_found=false;
        while ( (r==1) && (!state_end_found) && (!done) ) {
            xmlChar *localName=xmlTextReaderLocalName(reader), *localValue=NULL;
            depth = xmlTextReaderDepth(reader);
            switch ( xmlTextReaderNodeType( reader ) ) {
            case 15:
                state_end_found = (xmlStrEqual(localName, BAD_CAST section_name.c_str()) != 0);
                break;
            case 1:
                r = xmlTextReaderRead(reader);
                if (r==1) {
                    switch ( xmlTextReaderNodeType(reader) ) {
                    case 3:
                        if (!xmlTextReaderHasValue(reader))
                            throw std::runtime_error("Huh?! Expected a value now. This xml should maybe not be valid?!");
                        localValue = xmlTextReaderValue(reader);
                        name = string((char *)localName);
                        value = string((char *)localValue);
                        r = xmlTextReaderRead(reader);
                        done = true;
                        break;
                    case 14:
                        if (!xmlTextReaderHasValue(reader))
                            throw std::runtime_error("Huh?! Expected a value now. This xml should maybe not be valid?!");
                        localValue = xmlTextReaderValue(reader);
                        name = string((char *)localName);
                        value = string((char *)localValue);
                        r = xmlTextReaderRead(reader);
                        done = true;
                        subtree_found = true;
                    }
                }
            } // end of switch (type)
            xmlFree(localName);
            if (localValue!=NULL)
                xmlFree(localValue);
            if (!done)
                r=xmlTextReaderRead(reader);
        }

        if (r==0)
            state_end_found = true; // If r==0, it makes no sense to scan for more stuff...

        return subtree_found;
    }




    // should be replaced by the version in utils.hpp if that would work... argh...

    void pt_print0(model::StringStringPTree const& pt, const int level)
    {
        model::StringStringPTree::const_iterator end = pt.end();
        model::StringStringPTree::const_iterator it = pt.begin();
#if 0
        if (it==end) {
            for (int i=0; i<level; i++)
                cout << " ";
            cout << "(empty tree)" << endl;
        }
#endif
        for ( ; it != end; ++it) {
            for (int i=0; i<level; i++)
                cout << " ";
            cout << it->first << ": " << it->second.get_value<std::string>() << std::endl;
            pt_print0(it->second, level + 4);
        }
    }

    void pt_print(const string &msg, model::StringStringPTree const& pt)
    {
        printf("------------- ptree start ----------- (%s)\n", msg.c_str());
        pt_print0(pt, 0);
        printf("------------- ptree end -------------\n");
    }

    int pt_entries( model::StringStringPTree const& pt ) // Wouldn't it make sense if this was just pt.size()?!
    {
        const model::StringStringPTree::const_iterator end = pt.end();
        model::StringStringPTree::const_iterator it = pt.begin();
        int i;
        for (i=0; it!=end; it++) {
            i++;
        }
        return i;
    }




    namespace {

        void complexKeyFound( vector<string> &updatedKeys, const string &name, const model::StringStringPTree &ptree )
        {
            updatedKeys.push_back( name );
            pt_print("Found complex type", ptree);
        }

        void simpleKeyFound( vector<string> &updatedKeys, const string &name, const string &value )
        {
            updatedKeys.push_back( name );
            std::cout << "Found simple type, name and value: " << name << ": " << value << std::endl;
        }

    }

    vector<string> XMLReader::parseDocument(const xmlDocPtr doc,
                                            ElementHandler &elementHandler)
    {
        vector<string> updatedKeys;

        xmlTextReaderPtr reader = xmlReaderWalker(doc);
        if (reader==NULL)
            throw std::runtime_error("Could not create xml-reader.");

        int top_level_depth = gobbleUntilStartElement(reader, "State");
        if ( top_level_depth>=0 ) { // Ok, "State" was found, continue scanning for named elements...
            bool state_end_found=false, processing_complex_type=false;
            string complex_type_name;
            model::StringStringPTree ptree;
            ptree.add("root", 0);
            vector<model::StringStringPTree::iterator> prev_pos; // Recursion through stack-usage
            model::StringStringPTree::iterator current_pos = ptree.begin();
            int prev_depth = top_level_depth;

            while ( !state_end_found ) {

                string name, value;
                int depth;

                const bool subtree_detected = nextTypeContainsChildren(reader, "State", name, value, state_end_found, depth); // Remember that this call has side-effects...
                // A subtree has been found, we interpret this as the end of a complex type, if we are currently parsing one.

                if ( subtree_detected && (!state_end_found) && processing_complex_type ) {
                    // Subtree detected, but not state_end, while processing complex type... We should finish the current complex type and start a new one.
                    processing_complex_type = false;
                    elementHandler.updateElementFromPTree(complex_type_name, ptree.begin()->second);
                    complexKeyFound( updatedKeys, complex_type_name, ptree );
                    ptree.clear();
                    ptree.add("root", 0);
                    current_pos = ptree.begin();
                    prev_pos.clear();
                }

                if ( subtree_detected ) {
                    if (depth==top_level_depth+1) {
                        processing_complex_type = true; // We are at the root of a complex type.
                        complex_type_name = name;
                    }
                    current_pos->second.add(name, "contains fields on the level below, no value here");
                    prev_pos.push_back(current_pos);
                    current_pos = (--current_pos->second.end()); // "Recursive call"
                    prev_depth = depth;
                    continue;
                }

                bool restart = false;
                if (   ( (depth==top_level_depth+1) && (processing_complex_type) ) ||       // End of tree for complex type has been reached, even if no
                       (  state_end_found           && (processing_complex_type) )   ) {    // "state-end" has not been found.
                    processing_complex_type = false;
                    elementHandler.updateElementFromPTree(complex_type_name, ptree.begin()->second);
                    complexKeyFound( updatedKeys, complex_type_name, ptree );
                    ptree.clear();
                    ptree.add("root", 0);
                    current_pos = ptree.begin();
                    prev_pos.clear();
                    restart = true;
                }

                if ( !state_end_found ) {
                    if ( !restart ) {
                        if (depth<prev_depth) { // We leave the level and ascend. Must pop back to a previous position in the tree. ("Recursive return.")
                            for (int i=0; i<prev_depth-depth-1; i++)
                                prev_pos.pop_back();
                            current_pos = prev_pos.back();
                            prev_pos.pop_back();
                        }
                        if (processing_complex_type) { // We are building the tree for a complex type update, so add (name, val) to ptree.
                            current_pos->second.add(name, value);
                        }
                    }
                    if (!processing_complex_type) { // A simple type, call the appropriate method to update the state right away.
                        elementHandler.updateElementFromString(name, value);
                        simpleKeyFound( updatedKeys, name, value );
                    }
                }

                prev_depth = depth;

            } // end of "while not state_end_found" block
        }
        xmlFreeTextReader( reader );

        std::cout << "updated keys: ";
        for (int i=0; i<int(updatedKeys.size()); i++) {
            std::cout << updatedKeys[i] << " ";
        }
        std::cout << std::endl;

        return updatedKeys;
    }




}
}
}
}

















































