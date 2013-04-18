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
	ElementData::PropertyTree& ptree = elementData.getPropertyTree();
    ElementData widthED = createElement( value.width );
    ElementData heightED = createElement( value.height );
    ElementData timestampED = createElement( value.timestamp );
    ElementData sceneViewED = createElement( value.sceneView );
    ElementData projectionED = createMatrixElement( value.projectionMatrix.data() );
    ElementData modelviewED = createMatrixElement( value.modelviewMatrix.data() );

    ptree[ "width"] =  widthED;
    ptree[ "height"] =  heightED;
    ptree["projection"] =  projectionED;
    ptree["modelview"] =  modelviewED;
    ptree["timestamp"] =  timestampED;
    ptree["sceneView"] =  sceneViewED;

    return elementData;
}



template<>
inline
void
ElementDataFactory::createT<model::Viewer>( const ElementData& elementData, Viewer& t ) const {
    const ElementData::PropertyTree& ptree = elementData.getPropertyTree();

    createT( ptree.find("width")->second, t.width );
    createT( ptree.find("height")->second, t.height );
    createT( ptree.find("timestamp")->second , t.timestamp );
    createT( ptree.find( "sceneView")->second , t.sceneView );
    createMatrix( ptree.find("projection")->second, t.projectionMatrix.data() );
    createMatrix( ptree.find("modelview")->second, t.modelviewMatrix.data() );
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

    for(size_t i = 0; i < splitted.size(); ++i) {
        matrixData[i] = boost::lexical_cast<float>( splitted[i] ); 
    }
}


}
}
}

