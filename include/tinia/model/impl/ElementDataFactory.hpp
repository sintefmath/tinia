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

#include <vector>
#include <string>


#include "tinia/model/impl/ElementData.hpp"
#include "tinia/model/impl/TypeToXSDType.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>

namespace tinia {
namespace model {
namespace impl {

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
ElementDataFactory::createElement<model::Viewer>( const Viewer& value ) const {
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

    ptree.add( "width", widthED );
    ptree.add( "height", heightED );
    ptree.add( "projection", projectionED );
    ptree.add( "modelview", modelviewED );
    ptree.add( "timestamp", timestampED );
    ptree.add( "sceneView", sceneViewED );

    return elementData;
}



template<>
inline
void
ElementDataFactory::createT<model::Viewer>( const ElementData& elementData, Viewer& t ) const {
    const auto& ptree = elementData.getPropertyTree();

    createT( ptree.get<ElementData>( "width"  ), t.width );
    createT( ptree.get<ElementData>( "height" ), t.height );
    createT( ptree.get<ElementData>( "timestamp" ) , t.timestamp );
    createT( ptree.get<ElementData>( "sceneView" ) , t.sceneView );
    createMatrix( ptree.get<ElementData>( "projection"), t.projectionMatrix.data() );
    createMatrix( ptree.get<ElementData>( "modelview"), t.modelviewMatrix.data() );
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
ElementDataFactory::stringify<model::Viewer>( const Viewer& v ) const {
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
}

