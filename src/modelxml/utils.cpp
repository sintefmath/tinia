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

#include "tinia/policyxml/utils.hpp"

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

namespace tinia {
namespace policyxml {


xmlNodePtr xpathQuery( xmlDocPtr doc, std::string xpathExpression ) {
    auto xpathContext = xmlXPathNewContext( doc );

    xmlXPathRegisterNs( xpathContext, BAD_CAST "tns", BAD_CAST "http://cloudviz.sintef.no/V1/policy" );
    xmlXPathRegisterNs( xpathContext, BAD_CAST "xsd", BAD_CAST "http://www.w3.org/2001/XMLSchema" );

    auto xpathObject = xmlXPathEvalExpression( (xmlChar*)( xpathExpression.c_str() ), xpathContext );

    xmlNodePtr retval = 0;

    if ( xpathObject->nodesetval != 0 ) {
        if ( xpathObject->nodesetval->nodeNr != 0 ) {
            retval = xpathObject->nodesetval->nodeTab[0];
        }
    }

    xmlXPathFreeObject( xpathObject );
    xmlXPathFreeContext( xpathContext );

    return retval;
}


}

}









