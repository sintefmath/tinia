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

#include "tinia/model/ElementData.hpp"

#include <iostream>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>

using std::cout;
using std::endl;
using std::string;

namespace tinia {
namespace model {
const int ElementData::LENGTH_NOT_SET = -1;
const int ElementData::MATRIX_LENGTH = 16;

ElementData::ElementData()
:
  widgetType("textinput"),
  length( ElementData::LENGTH_NOT_SET )
{}


ElementData::ElementData(const ElementData &from)
   :
   // Deep copy
     stringValue(from.stringValue.begin(), from.stringValue.end()),
     xsdType(from.xsdType.begin(), from.xsdType.end()),
     widgetType( from.widgetType.begin(), from.widgetType.end()),
     minConstraint(from.minConstraint.begin(), from.minConstraint.end()),
     maxConstraint( from.maxConstraint.begin(), from.maxConstraint.end()),
     length(from.length)
{

   for(auto it = from.enumerationSet.begin();
       it != from.enumerationSet.end(); it++)
   {
      std::string inputString((*it).c_str());
      enumerationSet.insert(inputString);
   }

   for(auto it = from.annotationMap.begin();
       it != from.annotationMap.end(); it++)
   {
      annotationMap[it->first.c_str()] =it->second.c_str();
   }


   // TODO: Check if this is sensible enough
   if(from.propertyTree.get() != 0)
   {
      initializePropertyTree();
      for(auto it=from.propertyTree->begin(); it != from.propertyTree->end();
          it++)
      {
         propertyTree->push_back(
                  PropertyTree::value_type(it->first.c_str(), PropertyTree(ElementData(it->second.data()))
                                           ));
      }
   }

}

template<typename T>
bool ElementData::isWithinLimits(T& value, const std::string& stringValue) {
	if( !emptyRestrictionSet() ) {
		bool within = false;
		std::for_each(getEnumerationSet().begin(), getEnumerationSet().end(), [&stringValue, &within](const std::string allowed) {
			if( allowed == stringValue ) {
				within = true;
			}
		});
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
ElementData::setStringValue( std::string inputString ) {
    // printf("trying to set string to '%s'...\n", s.c_str()); fflush(stdout);

	// Check if it's compatible
	std::string xsdType = getXSDType();

	if( getLength() > 1 ) {
		std::vector<std::string> splitted;
		boost::split( splitted, inputString, boost::is_any_of(" ") );
		for( auto i = 0u; i < getLength(); i++  ) {
			checkValue( splitted[i] );
		}
	}
	else {
		checkValue( inputString );
	}
	
    stringValue = inputString;
}

void ElementData::checkValue(const std::string& s) {
	std::string xsdType = getXSDType();
	if(xsdType == "xsd:double") {
		double val = boost::lexical_cast<double>(s);
		if( !isWithinLimits(val, s) ) {
			throw std::runtime_error("Value = " + s + " is not within limits");
		}
	} 
	else if(xsdType == "xsd:float") {
		float val = boost::lexical_cast<float>(s);

		if( !isWithinLimits(val, s) ) {
			throw std::runtime_error("Value = " + s + " is not within limits");
		}
	} 
	else if(xsdType == "xsd:bool") {
		bool val = boost::lexical_cast<bool>(s);

		if( !isWithinLimits(val, s) ) {
			throw std::runtime_error("Value = " + s + " is not within limits");
		}
	}
	else if(xsdType == "xsd:integer") {
		int val = boost::lexical_cast<int>(s);

		if( !isWithinLimits(val, s) ) {
			throw std::runtime_error("Value = " + s + " is not within limits");
		}
	}
}




// in XMLReader.cpp, should be replaced by that in utils.hpp but it doesn't work?!?!?!?!?!!!!!
typedef boost::property_tree::basic_ptree<std::string, std::string> StringStringPTree;

void ElementData::setPropertyTreeValue_r( PropertyTree &pt, const StringStringPTree &sspt, const int level )
{
    if ( pt.size() != sspt.size() ) {
//        pt_print("properties from string-string-ptree", sspt);
        printf("pt.size()=%d\n", int(pt.size()));

        PropertyTree::iterator            end2  = pt.end();
        PropertyTree::iterator            it2  = pt.begin();
        for (; it2 != end2; it2++) {
            ElementData ed = it2->second.get_value<ElementData>();
            ed.print();
        }

        throw std::runtime_error("Huh?! The string-string ptree has a different topology than the string-ElementData ptree.");
    }
    StringStringPTree::const_iterator end  = sspt.end();
    PropertyTree::iterator            it2  = pt.begin();
    for (StringStringPTree::const_iterator it   = sspt.begin(); it != end; it++, it2++) {
        const string name = it->first;

        const string value = it->second.get_value<string>();

        // printf("setPropertyTreeValue_r: name=%s, value=%s\n", name.c_str(), value.c_str()); fflush(stdout);
        ElementData ed = it2->second.get_value<ElementData>();
        ed.setStringValue( value );
        it2->second.put_value(ed);
        setPropertyTreeValue_r(it2->second, it->second, level + 4);
    }
}


void ElementData::setPropertyTreeValue( const StringStringPTree &sspt )
{
    setPropertyTreeValue_r(*propertyTree, sspt, 0);
}


std::string
ElementData::getStringValue() const {
    return stringValue;
}

std::string
ElementData::getXSDType() const {
    return xsdType;
}

void
ElementData::setXSDType( std::string s ) {
    xsdType = s;
}

std::string
ElementData::getWidgetType() const {
    return widgetType;
}

void
ElementData::setWidgetType( std::string s ) {
    widgetType = s;
}

void
ElementData::setMinConstraint( std::string s ) {
    minConstraint = s;
}

std::string
ElementData::getMinConstraint() const {
    return minConstraint;
}

void
ElementData::setMaxConstraint( std::string s ) {
    maxConstraint = s;
}

std::string
ElementData::getMaxConstraint() const {
    return maxConstraint;
}

void
ElementData::setRestrictionSet( std::set<std::string>& restrictionSet ) {
    this->enumerationSet = restrictionSet;
    widgetType = "select";
}

const std::set<std::string>&
ElementData::getEnumerationSet() const {
    return enumerationSet;
}


bool
ElementData::emptyConstraints() const {
    return getMinConstraint().empty() && getMinConstraint().empty();
}

bool
ElementData::emptyRestrictionSet() const {
    return enumerationSet.empty();
}

bool
ElementData::emptyAnnotation() const {
    return annotationMap.empty();
}

void
ElementData::setAnnotation( std::unordered_map<std::string, std::string>& annotationMap ) {
    this->annotationMap = annotationMap;
}

const std::unordered_map<std::string, std::string>&
ElementData::getAnnotation() const {
    return annotationMap;
}

void
ElementData::setLength( unsigned int i ) {
    length = i;
}

int
ElementData::getLength() const {
    return length;
}

void
ElementData::initializePropertyTree() {
    propertyTree.reset( new PropertyTree() );
}

ElementData::PropertyTree&
ElementData::getPropertyTree() {
    if ( propertyTree.get() == 0 ) {
        initializePropertyTree();
    }

    return *propertyTree;
}

const ElementData::PropertyTree&
ElementData::getPropertyTree() const {
    if ( propertyTree.get() == 0 ) {
        throw std::runtime_error( "Trying the get a property tree but it is not initialized" );
    }
    return *propertyTree;
}


void ElementData::print0(const PropertyTree &pt, const int level) const
{
    PropertyTree::const_iterator end = pt.end();
    for (PropertyTree::const_iterator it = pt.begin(); it != end; ++it) {
        for (int i=0; i<level; i++)
            cout << " ";
        PropertyTree val = it->second;
        ElementData d = val.get_value<ElementData>();
        std::cout << it->first << ": " << d.getStringValue() << std::endl;
        //std::cout << it->first << ": " << "ugh" << std::endl;
        print0(it->second, level + 4);
    }
}


// For debugging
void ElementData::print(void) const
{
    printf("    xsdType=%s, widgetType=%s ", xsdType.c_str(), widgetType.c_str());
    const bool complextype = (xsdType.compare("xsd:complexType") == 0);
    if (complextype) {
        printf("[complex type]\n");
        print0(*propertyTree, 4);
    } else {
        printf("[simple type] \tval=%s\n", getStringValue().c_str());
        if (!emptyRestrictionSet()) {
            printf("        Restriction set:");
            const std::set<std::string> restr = getEnumerationSet();
            for( auto it = restr.begin(); it!=restr.end(); it++) {
                cout << " " << *it;
            }
            cout << endl;
        }
    }
    fflush(stdout);
}


}

bool model::ElementData::isComplexType() const
{
   // Primitive implementation
   return propertyTree.get() != NULL;
}

} // of namespace tinia































