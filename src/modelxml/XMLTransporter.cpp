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

#include <algorithm>
// QT's moc doesn't like BOOST_JOIN ( can be removed in QT 5.0 we think)
#ifndef Q_MOC_RUN 
#include <boost/lexical_cast.hpp>
#endif
#include <iostream>
#include <cstring>
#include <stdexcept>

#include "tinia/model/impl/xml/XMLTransporter.hpp"





namespace tinia {
namespace model {
namespace impl {
namespace xml {


using namespace std;

#ifdef USE_PERSISTENT_XML_BUFFER

    XMLTransporter::XMLTransporter():
        current_buf(NULL),
        current_buf_size(0),
        current_xmlBuffer(NULL),
        prev_buf(NULL),
        prev_buf_size(0)
    {
    }

    XMLTransporter::~XMLTransporter()
    {
        if (current_xmlBuffer!=NULL) {
            current_xmlBuffer->content = saved_content;
            current_xmlBuffer->size    = saved_size;
            current_xmlBuffer->alloc   = saved_allocScheme;
            current_xmlBuffer->use     = 0;
            xmlBufferFree(current_xmlBuffer);
            current_xmlBuffer = NULL; // Just in case...
        }
    }

    void XMLTransporter::xmlBufferCreateStaticMutable(char *buf, size_t buf_size)
    {
        // Note that an alternative to using xmlSaveFormatFileTo is to use the TextWriter api. See f.ex.
        // http://xmlsoft.org/examples/testWriter.c

        if ( (buf!=prev_buf) || (buf_size!=prev_buf_size) ) {
            // Creating a new xmlBuffer, since the input buffer signature has changed.
            prev_buf      = buf;
            prev_buf_size = buf_size;
            if (current_xmlBuffer!=NULL) {
                // Not first time creation, must destroy the old one properly.
                current_xmlBuffer->content = saved_content;
                current_xmlBuffer->size    = saved_size;
                current_xmlBuffer->alloc   = saved_allocScheme;
                current_xmlBuffer->use     = 0;
                xmlBufferFree(current_xmlBuffer);
            }
            current_xmlBuffer = xmlBufferCreate();
            if (current_xmlBuffer==NULL)
                throw std::runtime_error("Could not create current_xmlBuffer.");

            saved_content              = current_xmlBuffer->content;
            current_xmlBuffer->content = (xmlChar *)buf;
            current_xmlBuffer->use     = 0;
            saved_size                 = current_xmlBuffer->size;
            current_xmlBuffer->size    = buf_size;
            saved_allocScheme          = current_xmlBuffer->alloc;
            current_xmlBuffer->alloc   = XML_BUFFER_ALLOC_EXACT;
        } else {
            // Reusing the current_xmlBuffer.
            current_xmlBuffer->use=0;
        }

    }

#endif




    int XMLTransporter::writeXMLtoBuffer(xmlDocPtr doc, char *buf, size_t buf_size)
    {
#ifdef USE_PERSISTENT_XML_BUFFER
        xmlBufferCreateStaticMutable(buf, buf_size);
#else
        xmlBufferPtr current_xmlBuffer = xmlBufferCreate();
        if (current_xmlBuffer==NULL)
            throw std::runtime_error("Could not create xmlBuffer.");
#endif

        xmlOutputBufferPtr xmlOutBuf = xmlOutputBufferCreateBuffer(current_xmlBuffer, NULL); // xmlEncoder); // think NULL implies utf8
        if (xmlOutBuf==NULL)
            throw std::runtime_error("Could not create xmlOutputBuffer.");

        const int indent = 1; // Is this boolean? Or a bit field? Documentation does not say.
        const int bytes_written = xmlSaveFormatFileTo(xmlOutBuf,
                                                      doc,
                                                      NULL, // String for 'encoding' inserted into the xml header.
                                                      indent);
        if (bytes_written==-1)
            throw std::runtime_error( "Could not write XML document to buffer. Too small buffer?" );

        // According to the documentation, xmlOutBuf is now unavailable, but hopefully xmlBuffer is not.

        // Since we had to use xmlBufferCreate, we must now do a copy, and free the xmlBuffer.

        const unsigned mem_used = current_xmlBuffer->use;
        if (mem_used>buf_size)
            throw std::runtime_error("Buffer is too small for our data.");
        memcpy(buf, current_xmlBuffer->content, mem_used);
#ifndef USE_PERSISTENT_XML_BUFFER
        xmlBufferFree(current_xmlBuffer);
#endif
        return bytes_written;
    }




    xmlDocPtr XMLTransporter::readXMLfromBuffer(const char *buf, size_t doc_length)
    {
        xmlDocPtr doc = xmlReadMemory(buf, doc_length, "noname.xml", NULL, 0);
        if (doc == NULL)
            throw std::runtime_error("Could not parse buffer and generate XML.");
        return doc;
    }




    xmlDocPtr XMLTransporter::readXMLfromFile(const char *fname)
    {
        xmlDocPtr doc = xmlReadFile(fname, NULL, 0);
        if (doc == NULL)
            throw std::runtime_error("Could not parse file and generate XML.");
        return doc;
    }




}
}
}
}


























