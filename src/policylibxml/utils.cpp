#include "tinia/policylibxml/utils.hpp"

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

namespace tinia {
namespace policylibxml {


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









