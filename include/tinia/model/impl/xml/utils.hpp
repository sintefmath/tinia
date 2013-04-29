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
#include <string>
#include <stdexcept>
// QT's moc doesn't like BOOST_JOIN ( can be removed in QT 5.0 we think)
#ifndef Q_MOC_RUN 
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#endif
#include <libxml/tree.h>
#include <iostream>


/** \file utils.hpp
    This file contains various utilty function that is used throughout model.
    They are mostly intended for internal-use, but might also be convenient for
    other applications where libxml is used.
  */
namespace tinia {
namespace model {
namespace impl {
namespace xml {


/** Convenience function to do simple XPath queries. */
xmlNodePtr xpathQuery( xmlDocPtr doc, std::string xpathExpression );

/** Convert an xmlChar to type T. */
template<class T>
void xmlCharToType( xmlChar* c, T& t ) {
    t = boost::lexical_cast<T>( c );
}

/** Convenience function to get the value of an attribute converted to a type. */
template<class T>
void getXmlPropAsType( xmlNodePtr node, std::string property, T& t ) {
    if ( !xmlHasProp( node, BAD_CAST property.c_str() ) ) {
        throw std::runtime_error( "Node is missing property:" + property );
    }

    xmlChar* propValue = xmlGetProp( node, BAD_CAST property.c_str() );
    xmlCharToType( propValue, t );

    xmlFree( propValue );
}

/** Get the contents of a node converted to a type. */
template<class T>
void getXmlNodeContentAsType( xmlNodePtr node, T& t ) {
    xmlChar* content = xmlNodeGetContent( node );
    xmlCharToType( content, t );
    xmlFree( content );
}

template<typename U>
void print0(const boost::property_tree::basic_ptree<std::string, U> &pt, const int level) {
    //boost::property_tree::basic_ptree::const_iterator end = pt.end();
    typename boost::property_tree::basic_ptree<std::string, U>::const_iterator end = pt.end();
    //boost::property_tree::basic_ptree<T, U>::const_iterator it = pt.begin();
    auto it = pt.begin();
    for ( ; it != end; ++it) {
        for (int i=0; i<level; i++)
            std::cout << " ";
        U val = it->second;
        std::string valstr = "qwqw";
        //std::string valstr = it->second.get_value<std::string>();
        std::cout << it->first << ": " << valstr << std::endl;
        print0(it->second, level + 4);
    }
}

template<typename T, typename U>
void print(const boost::property_tree::basic_ptree<T, U> &pt) {
    std::cout << "------------- ptree -----------" << std::endl;
    print0(pt, 0);
    std::cout << "------------- ptree -----------" << std::endl;
}


}
}
}
}































