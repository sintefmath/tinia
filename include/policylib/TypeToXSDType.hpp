#ifndef TYPETOXSDTYPE_HPP
#define TYPETOXSDTYPE_HPP

#include <string>
#include "policylib/Viewer.hpp"
#include "policylib/File.hpp"

namespace policylib {

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
TypeToXSDType<policylib::Viewer>::getTypenameDetail() {
    return "complexType";
}

template<>
inline
std::string
TypeToXSDType<policylib::File>::getTypenameDetail() {
   return "file";
}

template<class T>
std::string
TypeToXSDType<T>::getTypenameDetail() {
    static_assert( sizeof(T) == 0, "By design you must add a specialization for your type." );
    return "";
}
}
#endif // TYPETOXSDTYPE_HPP
