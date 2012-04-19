#ifndef STATESCHEMAELEMENT_HPP
#define STATESCHEMAELEMENT_HPP
#include "policylib/ElementData.hpp"
#include <boost/property_tree/ptree.hpp>
#include <unordered_map>
#include <string>
namespace policylib {
class StateSchemaElement
{
public:
   typedef boost::property_tree::basic_ptree<std::string, StateSchemaElement> PropertyTree;

   StateSchemaElement();
   StateSchemaElement(std::string key, const ElementData data);

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
   const std::unordered_map<std::string, std::string>& getAnnotation() const;


   const std::unordered_set<std::string>& getEnumerationSet() const;

   /**
     Gets the enumeration set of the element.
     \param enumerationSet the set to put the result
     */
   template<typename T>
   void getEnumerationSet(std::unordered_set<T> &enumerationSet) const;

   /** Get the property tree. */
   PropertyTree
   getPropertyTree() const;


   /** Get the current length restriction.
     \return The current length restriction. Returns ElementData::LENGTH_NOT_SET when no length property is active.
     */
   int getLength() const;


   /** Default value of length field when the element does not have a length restriction. */
   static const int LENGTH_NOT_SET;

   /** Default matrix-length. */
   static const int MATRIX_LENGTH = 16;
private:
   std::string m_key;
   ElementData m_data;


};
}
#endif // STATESCHEMAELEMENT_HPP
