#include "policylib/StateSchemaElement.hpp"

namespace policylib {
const int StateSchemaElement::LENGTH_NOT_SET=-1;
StateSchemaElement::StateSchemaElement() {}
StateSchemaElement::StateSchemaElement(std::string key, const ElementData data)
   : m_key(key.c_str()), m_data(data)
{
}
}

std::string policylib::StateSchemaElement::getMaxConstraint() const
{
   return m_data.getMaxConstraint();
}

std::string policylib::StateSchemaElement::getMinConstraint() const
{
   return m_data.getMinConstraint();
}

bool policylib::StateSchemaElement::emptyConstraints() const
{
   return m_data.emptyConstraints();
}

bool policylib::StateSchemaElement::emptyRestrictionSet() const
{
   return m_data.emptyRestrictionSet();
}

bool policylib::StateSchemaElement::emptyAnnotation() const
{
   return m_data.emptyAnnotation();
}

const std::unordered_map<std::string, std::string> &
policylib::StateSchemaElement::getAnnotation() const
{
   return m_data.getAnnotation();
}

const std::unordered_set<std::string> & policylib::StateSchemaElement::getEnumerationSet() const
{
   return m_data.getEnumerationSet();
}

template<typename T>
void policylib::StateSchemaElement::getMaxConstraint(T& t) const
{
   //TOOD Write me
}

template<typename T>
void policylib::StateSchemaElement::getMinConstraint(T& t) const
{
   //TOOD Write me
}

template<typename T>
void policylib::StateSchemaElement::getEnumerationSet(std::unordered_set<T> &enumerationSet) const
{
   // TODO Write me
}

policylib::StateSchemaElement::PropertyTree
   policylib::StateSchemaElement::getPropertyTree() const
{
   // TODO Make this more effective by implementing a "build on demand"-tree?
   // TODO Make this function return by reference (i.e. make destPropertyTree
   // an object member)?
   policylib::StateSchemaElement::PropertyTree destPropertyTree;

   const policylib::ElementData::PropertyTree& sourcePropertyTree  = m_data.getPropertyTree();
   for(auto it = sourcePropertyTree.begin(); it!=sourcePropertyTree.end();
       it++)
   {
      destPropertyTree.push_back(PropertyTree::value_type(it->first,
                                 PropertyTree(StateSchemaElement(it->first, it->second.data()))));
   }

   return destPropertyTree;
}

int policylib::StateSchemaElement::getLength() const
{
   return m_data.getLength();
}

std::string policylib::StateSchemaElement::getKey() const
{
   return m_key;
}

std::string policylib::StateSchemaElement::getXSDType() const
{
   return m_data.getXSDType();
}

std::string policylib::StateSchemaElement::getWidgetType() const
{
   return m_data.getWidgetType();
}
