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

#ifndef TESTUTILS_HPP
#define TESTUTILS_HPP

#include "tinia/modelxml/XMLHandler.hpp"
#include "libxml/tree.h"

struct TestHelper {
    template<class T>
    TestHelper(tinia::modelxml::XMLHandler& handler , T t, bool printDoc = false ) {
        doc = handler.getCompleteDocument();
        if ( printDoc ) {
            xmlSaveFormatFile( "-", doc, 1 );
        }
        t( doc, doc->children );
    }

    ~TestHelper() {
        xmlFreeDoc( doc );

    }

    xmlDocPtr doc;
};

#endif // TESTUTILS_HPP
