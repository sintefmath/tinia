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

#ifndef TYPETOXSDTYPE_HPP
#define TYPETOXSDTYPE_HPP

#include <string>
#include "tinia/model/Viewer.hpp"
#include "tinia/model/File.hpp"


namespace tinia {
namespace model {
namespace impl {

/** \class TypeToXSDType
    TypeToXSDType is responsible for representing the mapping between C++ types and the XML Schema type
    identificator, such as xsd:integer.

    The mapping is done at by explicit template specialization of the getTypenameDetail static member function.
  */
template<class T>
class TypeToXSDType {
public:
    /** Function to get the XML Schema type-name for a given C++ type.
      \param ns Namespace for xml schema. The default should normally be used.
      \return String describing the the XML-schema type of T.
      */
    static std::string getTypename( std::string ns = "xsd" );
private:    
    static std::string getTypenameDetail();
};

template<class T>
std::string
TypeToXSDType<T>::getTypename( std::string ns ) {
    return ns + ":" + getTypenameDetail();
}


template<>
inline
std::string
TypeToXSDType<int>::getTypenameDetail() {
    return "integer";
}

template<>
inline
std::string
TypeToXSDType<float>::getTypenameDetail() {
    return "float";
}

template<>
inline
std::string
TypeToXSDType<double>::getTypenameDetail() {
    return "double";
}

template<>
inline
std::string
TypeToXSDType<bool>::getTypenameDetail() {
    return "bool";
}


template<>
inline
std::string
TypeToXSDType<std::string>::getTypenameDetail() {
    return "string";
}

template<>
inline
std::string
TypeToXSDType<char*>::getTypenameDetail() {
    return "string";
}

template<>
inline
std::string
TypeToXSDType<const char*>::getTypenameDetail() {
    return "string";
}

template<>
inline
std::string
TypeToXSDType<model::Viewer>::getTypenameDetail() {
    return "complexType";
}

template<>
inline
std::string
TypeToXSDType<model::File>::getTypenameDetail() {
   return "file";
}

template<class T>
std::string
TypeToXSDType<T>::getTypenameDetail() {
    static_assert( sizeof(T) == 0, "By design you must add a specialization for your type." );
    return "";
}
}
}
}
#endif // TYPETOXSDTYPE_HPP
