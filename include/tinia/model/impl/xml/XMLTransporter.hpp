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

#pragma once
#include <unordered_map>
#include <libxml/tree.h>
#include "tinia/model/impl/ElementData.hpp"

#include "tinia/model/impl/xml/utils.hpp"



#define USE_PERSISTENT_XML_BUFFER



namespace tinia {
namespace model {
namespace impl {
namespace xml {


/** \class XMLTransporter
     XMLTransporter is responsible for writing an xmlDocPtr to a buffer, or reading a buffer into such a pointer.
  */
class XMLTransporter
{

public:

    /** Write the specified XML-document to a buffer. Return value is the bytes written to the buffer.
     \param doc The XML document.
     \param buf The buffer, this should be allocated already.
     \param buf_size Maximum available characters in the buffer.
     \throws std::runtime_error if something goes wrong.
     */
    int writeXMLtoBuffer(xmlDocPtr doc, char *buf, size_t buf_size);

    /** Read an XML document from the specified buffer. The document is returned.
     \param buf The buffer.
     \param doc_length Maximum available characters in the buffer. (The document may use a smaller number of characters.)
     \throws std::runtime_error if something goes wrong.
     */
    xmlDocPtr readXMLfromBuffer(const char *buf, size_t doc_length);

    xmlDocPtr readXMLfromFile(const char *fname);

#ifdef USE_PERSISTENT_XML_BUFFER

    XMLTransporter();
    ~XMLTransporter();

private:

    char *current_buf;
    size_t current_buf_size;
    xmlBufferPtr current_xmlBuffer;

    char *prev_buf;
    size_t prev_buf_size;
    xmlChar* saved_content;
    size_t saved_size;
    xmlBufferAllocationScheme saved_allocScheme;

    /** Used internally, if the parameters correspond to the values of the current xmlBuffer nothing is done, otherwise
      the old xmlBuffer is destroyed and a new created.
      The name is selected to match what would seem like a nice thing to have in libxml2. (See xmlBufferCreateStatic);
      */
    void xmlBufferCreateStaticMutable( char *buf, const size_t buf_size );

#endif

};

}
}
}
}
































