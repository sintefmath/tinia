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

#include <map>
#include <string>
#include <memory>
#include <iostream>
// QT's moc doesn't like BOOST_JOIN ( can be removed in QT 5.0 we think)
#ifndef Q_MOC_RUN 
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/lexical_cast.hpp>
#endif
#include <set>

#include "tinia/model/Viewer.hpp"


namespace tinia {
namespace model {
namespace impl {

/** \class ElementData is responsible for storing data and metadata about a state parameter.
    This class was designed to be a helper class for ExposedModel and is not designed to be used as a standalone-class.
    */
class ElementData {
public:
    typedef ElementData SelfType;
    typedef boost::property_tree::basic_ptree<std::string, std::string> StringStringPTree;
    //typedef boost::property_tree::basic_ptree<std::string, SelfType>    PropertyTree;
    typedef std::map<std::string, SelfType> PropertyTree;

    ElementData();

    //ElementData(const ElementData& from);

    //ElementData& operator=(const ElementData& from);
    /** Set the value of the data represented as a string. */
    void setStringValue( std::string );

    /** Set the value of the data represented as a property tree. */
    void setPropertyTreeValue_r( std::map<std::string, SelfType> &pt, const StringStringPTree &sspt, const int level );
    void setPropertyTreeValue( const StringStringPTree &sspt );

    /** Get the value of the data represented as a string. */
    std::string getStringValue() const;

    /** Set the XSD type of the data. */
    void setXSDType( std::string );

    /** Get the XSD type of the data. */
    std::string getXSDType() const;

    /** Set the widget type of the data. */
    void setWidgetType( std::string );

    /** Get the widget type of the data. */
    std::string getWidgetType() const;

    /** Set the minimum constrait of the data, this is normally a number formatted as a string,
        but anything with sensible comparison operators might work.
        */
    void setMinConstraint( std::string );

    /** Get the minimum constraint of the data. */
    std::string getMinConstraint() const;


    /** Set the maximum constrait of the data, this is normally a number formatted as a string,
        but anything with sensible comparison operators might work.
        */
    void setMaxConstraint( std::string );

    /** Get the maximum constraint of the data. */
    std::string getMaxConstraint() const;

    /** Get the maximum constraint of the data. */
    template<typename T>
    void getMaxConstraint(T& t) const;

    /** Get the minimum constraint of the data. */
    template<typename T>
    void getMinConstraint(T& t) const;

    /** Return true if the element has no constraints. */
    bool emptyConstraints() const;

    /** Return true if value is outside the constraints in elementData. */
    template<typename T>
    bool invalidConstraints( const T& value ) const;

    /** Return true if value is violating the restrictions in elementData. */
    template<typename T>
    bool violatingRestriction( const T& value ) const;

    /** Return true if the element has no restrictions. */
    bool emptyRestrictionSet() const;

    /** Set the restriction set. */
    void setRestrictionSet( std::set<std::string>& restrictionSet );

    /** Get a reference to the enumerationSet. */
    const std::set<std::string>& getEnumerationSet() const;

    /** Return true if there is no annotation set for the element. */
    bool emptyAnnotation() const;

    /** Set the annotation for the element.
      \param annotationMap Contains mapping between language code and annotation string.
             Eg: ["en"]->"An annotation", ["no"]->"En beskrivelse"
      */
    void setAnnotation( std::map<std::string, std::string>& annotationMap);

    /** Get the annotation of the element. */
    const std::map<std::string, std::string>& getAnnotation() const;

    /** Get the revision number for the previous changeof this element. */
    unsigned int getRevisionNumber() const { return preChangeRevisionNumber; }

    /** Update the revision number for the element. */
    void setRevisionNumber(const int rn) { preChangeRevisionNumber = rn; }

    /** Set the length restriction of the element. Used internally to represent matrices.
        Set to ElementData::LENGTH_NOT_SET to to  remove length restriction.
      */
    void setLength( unsigned int );

    /** Get the current length restriction.
      \return The current length restriction. Returns ElementData::LENGTH_NOT_SET when no length property is active.
      */
    int getLength() const;

    /** Get the property tree. */
    std::map<std::string, SelfType>& getPropertyTree();

    const PropertyTree& getPropertyTree() const;

    /**
      Returns true if this is a complex type (in other word, if its value
      is represented by a PropertyTree
      */
    bool isComplexType() const;

public:
    void print(void) const;

    /** Default value of length field when the element does not have a length restriction. */
    static const int LENGTH_NOT_SET;

    /** Default matrix-length. */
    static const int MATRIX_LENGTH;
private:
	void checkValue(const std::string& stringValue);
	template<typename T>
	bool isWithinLimits(T& value, const std::string& stringValue);

    std::string stringValue;
    std::string xsdType;
    std::string widgetType;
    std::string minConstraint;
    std::string maxConstraint;
    std::set<std::string> enumerationSet;
	std::map<std::string, std::string> annotationMap;
    int preChangeRevisionNumber; // The revision number just prior to updating this element
    int length;

    std::map<std::string, SelfType> propertyTree;

};


template<>
inline
bool
ElementData::invalidConstraints<const char*>( const char* const& value ) const {
    return false;
}
template<typename T>
bool
ElementData::invalidConstraints( const T& value ) const {

    if ( emptyConstraints() ) {
        return false;
    }

    const T minValue = boost::lexical_cast<T>( getMinConstraint() );
    const T maxValue = boost::lexical_cast<T>( getMaxConstraint() );

    if ( ( minValue <= value ) && ( value <= maxValue ) ) {
        return false;
    }

    return true;
}

template<typename T>
bool
ElementData::violatingRestriction( const T& value ) const {
    if ( emptyRestrictionSet() ) {
        return false;
    }

    std::string sValue = boost::lexical_cast<std::string>( value );
    const std::set<std::string>& restrictionSet = getEnumerationSet();

    return restrictionSet.find( sValue ) == restrictionSet.end();
}

/** Get the maximum constraint of the data. */
template<typename T>
void
ElementData::getMaxConstraint(T& t) const {
    t = boost::lexical_cast<T>(getMaxConstraint());
}

/** Get the minimum constraint of the data. */
template<typename T>
void
ElementData::getMinConstraint(T& t) const {
    t = boost::lexical_cast<T>(getMinConstraint());
}
}
}
}
