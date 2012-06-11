#include <tinia/policy/StateSchemaElement.hpp>

namespace tinia {
namespace policy {
const int StateSchemaElement::LENGTH_NOT_SET=-1;
StateSchemaElement::StateSchemaElement() {}
StateSchemaElement::StateSchemaElement(std::string key, const ElementData data)
   : m_key(key.c_str()), m_data(data)
{
}
}

std::string policy::StateSchemaElement::getMaxConstraint() const
{
   return m_data.getMaxConstraint();
}

std::string policy::StateSchemaElement::getMinConstraint() const
{
   return m_data.getMinConstraint();
}

bool policy::StateSchemaElement::emptyConstraints() const
{
   return m_data.emptyConstraints();
}

bool policy::StateSchemaElement::emptyRestrictionSet() const
{
   return m_data.emptyRestrictionSet();
}

bool policy::StateSchemaElement::emptyAnnotation() const
{
   return m_data.emptyAnnotation();
}

const std::unordered_map<std::string, std::string> &
policy::StateSchemaElement::getAnnotation() const
{
   return m_data.getAnnotation();
}

const std::unordered_set<std::string> & policy::StateSchemaElement::getEnumerationSet() const
{
   return m_data.getEnumerationSet();
}



template<typename T>
void policy::StateSchemaElement::getEnumerationSet(std::unordered_set<T> &enumerationSet) const
{
   // TODO Write me
}

policy::StateSchemaElement::PropertyTree
   policy::StateSchemaElement::getPropertyTree() const
{
   // TODO Make this more effective by implementing a "build on demand"-tree?
   // TODO Make this function return by reference (i.e. make destPropertyTree
   // an object member)?
   policy::StateSchemaElement::PropertyTree destPropertyTree;

   const policy::ElementData::PropertyTree& sourcePropertyTree  = m_data.getPropertyTree();
   for(auto it = sourcePropertyTree.begin(); it!=sourcePropertyTree.end();
       it++)
   {
      destPropertyTree.push_back(PropertyTree::value_type(it->first,
                                 PropertyTree(StateSchemaElement(it->first, it->second.data()))));
   }

   return destPropertyTree;
}

int policy::StateSchemaElement::getLength() const
{
   return m_data.getLength();
}

std::string policy::StateSchemaElement::getKey() const
{
   return m_key;
}

std::string policy::StateSchemaElement::getXSDType() const
{
   return m_data.getXSDType();
}

std::string policy::StateSchemaElement::getWidgetType() const
{
   return m_data.getWidgetType();
}
} // of namespace tinia
