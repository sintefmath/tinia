#ifndef ELEMENTDATA_HPP
#define ELEMENTDATA_HPP

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/lexical_cast.hpp>

#include "policylib/Viewer.hpp"


namespace policylib {

/** \class ElementData is responsible for storing data and metadata about a state parameter.
    This class was designed to be a helper class for PolicyLib and is not designed to be used as a standalone-class.
    */
class ElementData {
public:
    typedef ElementData SelfType;
    typedef boost::property_tree::basic_ptree<std::string, std::string> StringStringPTree;
    typedef boost::property_tree::basic_ptree<std::string, SelfType>    PropertyTree;

    ElementData();

    ElementData(const ElementData& from);
    /** Set the value of the data represented as a string. */
    void setStringValue( std::string );

    /** Set the value of the data represented as a property tree. */
    void setPropertyTreeValue_r( PropertyTree &pt, const StringStringPTree &sspt, const int level );
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
    void setRestrictionSet( std::unordered_set<std::string>& restrictionSet );

    /** Get a reference to the enumerationSet. */
    const std::unordered_set<std::string>& getEnumerationSet() const;

    /** Return true if there is no annotation set for the element. */
    bool emptyAnnotation() const;

    /** Set the annotation for the element.
      \param annotationMap Contains mapping between language code and annotation string.
             Eg: ["en"]->"An annotation", ["no"]->"En beskrivelse"
      */
    void setAnnotation( std::unordered_map<std::string, std::string>& annotationMap);

    /** Get the annotation of the element. */
    const std::unordered_map<std::string, std::string>& getAnnotation() const;

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
    PropertyTree& getPropertyTree();

    const PropertyTree& getPropertyTree() const;

    /**
      Returns true if this is a complex type (in other word, if its value
      is represented by a PropertyTree
      */
    bool isComplexType() const;

private:
    void print0(const PropertyTree &pt, const int level) const;
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
    void initializePropertyTree();

    std::string stringValue;
    std::string xsdType;
    std::string widgetType;
    std::string minConstraint;
    std::string maxConstraint;
    std::unordered_set<std::string> enumerationSet;
    std::unordered_map<std::string, std::string> annotationMap;
    int preChangeRevisionNumber; // The revision number just prior to updating this element
    int length;

    // Should ideally a a unique_ptr but it fails at compile time for templatical-reasons.
    std::shared_ptr<PropertyTree> propertyTree;
    //PropertyTree propertyTree;
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
    auto& restrictionSet = getEnumerationSet();

    return restrictionSet.find( sValue ) == restrictionSet.end();
}



}
#endif // ELEMENTDATA_HPP
