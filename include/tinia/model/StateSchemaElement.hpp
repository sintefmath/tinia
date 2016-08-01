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
#include "tinia/model/impl/ElementData.hpp"
// QT's moc doesn't like BOOST_JOIN ( can be removed in QT 5.0 we think)
#ifndef Q_MOC_RUN 
#include <boost/property_tree/ptree.hpp>
#endif
#include <string>

namespace tinia {
namespace model {
class StateSchemaElement
{
public:
   typedef impl::ElementData::PropertyTree PropertyTree;

   StateSchemaElement();
   StateSchemaElement(std::string key, const impl::ElementData data);

   /** Gets the key(name) of the element**/
   std::string getKey() const;

   /** Gets the type of the element **/
   std::string getXSDType() const;

   /** Gets a hint as to how the element could be presented in a GUI */
   std::string getWidgetType() const;


   std::string getMaxConstraint() const;

   std::string getMinConstraint() const;

   /** Get the maximum constraint of the data. */
   template<typename T>
   void getMaxConstraint(T& t) const;

   /** Get the minimum constraint of the data. */
   template<typename T>
   void getMinConstraint(T& t) const;

   /** \return True if it does not have a constraint, false otherwise.*/
   bool emptyConstraints() const;

   /** \return True if it does not have a restriction set, false otherwise. */
   bool emptyRestrictionSet() const;

   /** \return True if it does not have an annotation, false otherwise. */
   bool emptyAnnotation() const;

   /** \return The annotation of the element */
   const std::map<std::string, std::string>& getAnnotation() const;


   const std::set<std::string>& getEnumerationSet() const;

   /**
     Gets the enumeration set of the element.
     \param enumerationSet the set to put the result
     */
   template<typename T>
   void getEnumerationSet(std::set<T> &enumerationSet) const;

   /** Get the property tree. */
   PropertyTree
   getPropertyTree() const;


   /** Get the current length restriction.
     \return The current length restriction. Returns impl::ElementData::LENGTH_NOT_SET when no length property is active.
     */
   int getLength() const;


   /** Default value of length field when the element does not have a length restriction. */
   static const int LENGTH_NOT_SET;

   /** Default matrix-length. */
   static const int MATRIX_LENGTH = 16;
private:
   std::string m_key;
   impl::ElementData m_data;


};

template<typename T>
void StateSchemaElement::getMaxConstraint(T& t) const
{
    m_data.getMaxConstraint(t);
}

template<typename T>
void model::StateSchemaElement::getMinConstraint(T& t) const
{
    m_data.getMinConstraint(t);
}
}
}

