#ifndef ELEMENTDATAFACTORY_HPP
#define ELEMENTDATAFACTORY_HPP

#include <vector>
#include <string>


#include "tinia/policylib/ElementData.hpp"
#include "tinia/policylib/TypeToXSDType.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>

namespace tinia {
namespace policylib {

/** \class ElementDataFactory
    ElementDataFactory is responsible for creating ElementData objects for various types.
 */
class ElementDataFactory {
public:
    template<class T>
    ElementData createElement( const T& t ) const;

    ElementData createMatrixElement( const float* matrixData ) const;

    template<class T>
    void createT( const ElementData& elementData, T& t ) const;

    void createMatrix( const ElementData& elementData, float* matrixData ) const;

private:
    template<class T>
    std::string stringify( const T& t ) const;
};


template<>
inline
ElementData
ElementDataFactory::createElement<policylib::Viewer>( const Viewer& value ) const {
    ElementData elementData;

    elementData.setXSDType( "xsd:complexType" );
    elementData.setWidgetType( "canvas" );
    auto& ptree = elementData.getPropertyTree();
    auto widthED = createElement( value.width );
    auto heightED = createElement( value.height );
    auto timestampED = createElement( value.timestamp );
    auto sceneViewED = createElement( value.sceneView );
    auto projectionED = createMatrixElement( value.projectionMatrix.data() );
    auto modelviewED = createMatrixElement( value.modelviewMatrix.data() );

    ptree.add( "Width", widthED );
    ptree.add( "Height", heightED );
    ptree.add( "Projection", projectionED );
    ptree.add( "Modelview", modelviewED );
    ptree.add( "Timestamp", timestampED );
    ptree.add( "SceneView", sceneViewED );

    return elementData;
}



template<>
inline
void
ElementDataFactory::createT<policylib::Viewer>( const ElementData& elementData, Viewer& t ) const {
    const auto& ptree = elementData.getPropertyTree();

    createT( ptree.get<ElementData>( "Width"  ), t.width );
    createT( ptree.get<ElementData>( "Height" ), t.height );
    createT( ptree.get<ElementData>( "Timestamp" ) , t.timestamp );
    createT( ptree.get<ElementData>( "SceneView" ) , t.sceneView );
    createMatrix( ptree.get<ElementData>( "Projection"), t.projectionMatrix.data() );
    createMatrix( ptree.get<ElementData>( "Modelview"), t.modelviewMatrix.data() );
}

inline
ElementData
ElementDataFactory::createMatrixElement( const float* matrixData ) const {
    std::stringstream ss;
    // Doing 15 elements to get rid of the pesky whitespace at end of string.
    copy( matrixData, matrixData + ElementData::MATRIX_LENGTH - 1, std::ostream_iterator<float>( ss, " " ) );
    ss << *(matrixData + ElementData::MATRIX_LENGTH - 1 );

    ElementData elementData;
    elementData.setXSDType( "xsd:float" );
    elementData.setLength( ElementData::MATRIX_LENGTH );
    elementData.setStringValue( ss.str() );

    return elementData;
}

template<class T>
ElementData
ElementDataFactory::createElement( const T& value ) const {
    ElementData elementData;

    elementData.setStringValue( stringify( value ) );
    elementData.setXSDType( TypeToXSDType<T>::getTypename() );

    return elementData;
}

template<class T>
void
ElementDataFactory::createT( const ElementData& elementData, T& t ) const {
    t = boost::lexical_cast<T>( elementData.getStringValue() );
}

template<>
inline
std::string
ElementDataFactory::stringify<policylib::Viewer>( const Viewer& v ) const {
    return "";
}

template<class T>
std::string
ElementDataFactory::stringify( const T& t ) const  {
    std::stringstream ss;
    ss << t;

    return ss.str();
}

inline
void
ElementDataFactory::createMatrix( const ElementData& elementData, float* matrixData ) const {
    std::vector<std::string> splitted;
    std::string s( elementData.getStringValue() );
    boost::split( splitted, s, boost::is_any_of(" ") );

    std::transform( splitted.begin(), splitted.end(), matrixData, []( const std::string& s ) {
              return boost::lexical_cast<float>( s ); }
    );
}



}
}
#endif // ELEMENTDATAFACTORY_HPP
