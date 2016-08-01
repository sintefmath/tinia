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

#include "tinia/model/impl/ElementData.hpp"

#include <iostream>
#include <string>
#include <vector>
// QT's moc doesn't like BOOST_JOIN ( can be removed in QT 5.0 we think)
#ifndef Q_MOC_RUN 
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#endif
#include <tinia/model/exceptions/RestrictionException.hpp>

using std::cout;
using std::endl;
using std::string;

namespace tinia {
namespace model {
const int impl::ElementData::LENGTH_NOT_SET = -1;
const int impl::ElementData::MATRIX_LENGTH = 16;

impl::ElementData::ElementData()
:
  widgetType("textinput"),
  length( impl::ElementData::LENGTH_NOT_SET )
{}

template<typename T>
bool impl::ElementData::isWithinLimits(T& value, const std::string& stringValue) {
	if( !emptyRestrictionSet() ) {
		bool within = false;
		for(std::set<std::string>::const_iterator it = getEnumerationSet().begin(); it !=  getEnumerationSet().end(); ++it) {
			const std::string& allowed = *it;
			if( allowed == stringValue ) {
				within = true;
			}
		}
		return within;
	}

	if( !emptyConstraints() ) {
		T minValue = boost::lexical_cast<T>(getMinConstraint());
		T maxValue = boost::lexical_cast<T>(getMaxConstraint());

		return value <= maxValue && value >= minValue;
	}

	return true;
}

void
impl::ElementData::setStringValue( std::string inputString ) {
    // printf("trying to set string to '%s'...\n", s.c_str()); fflush(stdout);

	// Check if it's compatible
	std::string xsdType = getXSDType();

	if( getLength() > 1 ) {
		std::vector<std::string> splitted;
		boost::split( splitted, inputString, boost::is_any_of(" ") );
        for( int i = 0; i < getLength(); i++  ) {
			checkValue( splitted[i] );
		}
	}
	else {
		checkValue( inputString );
	}
	
    stringValue = inputString;
}

void impl::ElementData::checkValue(const std::string& s) {
	std::string xsdType = getXSDType();
	if(xsdType == "xsd:double") {
		double val = boost::lexical_cast<double>(s);
		if( !isWithinLimits(val, s) ) {
            throw RestrictionException(s);
		}
	} 
	else if(xsdType == "xsd:float") {
		float val = boost::lexical_cast<float>(s);

		if( !isWithinLimits(val, s) ) {
            throw RestrictionException(s);
		}
	} 
	else if(xsdType == "xsd:bool") {
		bool val = boost::lexical_cast<bool>(s);

		if( !isWithinLimits(val, s) ) {
            throw RestrictionException(s);
		}
	}
	else if(xsdType == "xsd:integer") {
		int val = boost::lexical_cast<int>(s);

		if( !isWithinLimits(val, s) ) {
            throw RestrictionException(s);
		}
	}
}




// in XMLReader.cpp, should be replaced by that in utils.hpp but it doesn't work?!?!?!?!?!!!!!
typedef boost::property_tree::basic_ptree<std::string, std::string> StringStringPTree;

void impl::ElementData::setPropertyTreeValue_r( std::map<std::string, impl::ElementData> &pt, const StringStringPTree &sspt, const int level )
{
    if ( pt.size() != sspt.size() ) {
        throw std::runtime_error("Huh?! The string-string ptree has a different topology than the string-impl::ElementData ptree.");
    }
    StringStringPTree::const_iterator end  = sspt.end();
    std::map<std::string, impl::ElementData>::iterator it2  = pt.begin();
    for (StringStringPTree::const_iterator it   = sspt.begin(); it != end; it++, it2++) {
        const string name = it->first;

        const string value = it->second.get_value<string>();

        // printf("setPropertyTreeValue_r: name=%s, value=%s\n", name.c_str(), value.c_str()); fflush(stdout);
        pt[it->first].setStringValue(value);

        setPropertyTreeValue_r(it2->second.propertyTree, it->second, level + 4);
    }
}


void impl::ElementData::setPropertyTreeValue( const StringStringPTree &sspt )
{
    setPropertyTreeValue_r(propertyTree, sspt, 0);
}


std::string
impl::ElementData::getStringValue() const {
    return stringValue;
}

std::string
impl::ElementData::getXSDType() const {
    return xsdType;
}

void
impl::ElementData::setXSDType( std::string s ) {
    xsdType = s;
}

std::string
impl::ElementData::getWidgetType() const {
    return widgetType;
}

void
impl::ElementData::setWidgetType( std::string s ) {
    widgetType = s;
}

void
impl::ElementData::setMinConstraint( std::string s ) {
    minConstraint = s;
}

std::string
impl::ElementData::getMinConstraint() const {
    return minConstraint;
}

void
impl::ElementData::setMaxConstraint( std::string s ) {
    maxConstraint = s;
}

std::string
impl::ElementData::getMaxConstraint() const {
    return maxConstraint;
}

void
impl::ElementData::setRestrictionSet( std::set<std::string>& restrictionSet ) {
    this->enumerationSet = restrictionSet;
    widgetType = "select";
}

const std::set<std::string>&
impl::ElementData::getEnumerationSet() const {
    return enumerationSet;
}


bool
impl::ElementData::emptyConstraints() const {
    return getMinConstraint().empty() && getMinConstraint().empty();
}

bool
impl::ElementData::emptyRestrictionSet() const {
    return enumerationSet.empty();
}

bool
impl::ElementData::emptyAnnotation() const {
    return annotationMap.empty();
}

void
impl::ElementData::setAnnotation( std::map<std::string, std::string>& annotationMap ) {
    this->annotationMap = annotationMap;
}

const std::map<std::string, std::string>&
impl::ElementData::getAnnotation() const {
    return annotationMap;
}

void
impl::ElementData::setLength( unsigned int i ) {
    length = i;
}

int
impl::ElementData::getLength() const {
    return length;
}


impl::ElementData::PropertyTree&
impl::ElementData::getPropertyTree() {

    return propertyTree;
}

const impl::ElementData::PropertyTree&
impl::ElementData::getPropertyTree() const {
    return propertyTree;
}



// For debugging
void impl::ElementData::print(void) const
{
    printf("    xsdType=%s, widgetType=%s ", xsdType.c_str(), widgetType.c_str());
    const bool complextype = (xsdType.compare("xsd:complexType") == 0);
    if (complextype) {
        printf("[complex type]\n");
        //print0(*propertyTree, 4);
    } else {
        printf("[simple type] \tval=%s\n", getStringValue().c_str());
        if (!emptyRestrictionSet()) {
            printf("        Restriction set:");
            const std::set<std::string> restr = getEnumerationSet();
            for( std::set<std::string>::const_iterator it = restr.begin(); it!=restr.end(); it++) {
                cout << " " << *it;
            }
            cout << endl;
        }
    }
    fflush(stdout);
}


}

bool model::impl::ElementData::isComplexType() const
{
   // Primitive implementation
   return propertyTree.size() > 0;
}

} // of namespace tinia































