#include "tinia/policy/StateElement.hpp"
namespace tinia {
policy::StateElement::StateElement(std::string name, const ElementData &data)
   : m_name(name.c_str()), m_data(data)
{

}

policy::StateElement::StateElement() {}

std::string policy::StateElement::getKey() const
{
   return m_name;
}


std::string policy::StateElement::getXSDType() const
{
   return m_data.getXSDType();
}

std::string policy::StateElement::getStringValue() const
{
   return m_data.getStringValue();
}

policy::StateElement::PropertyTree policy::StateElement::getPropertyTree() const
{
   // TODO Make this more effective by implementing a "build on demand"-tree?
   // TODO Make this function return by reference (i.e. make destPropertyTree
   // an object member)?
   PropertyTree destPropertyTree;

   const ElementData::PropertyTree& sourcePropertyTree  = m_data.getPropertyTree();
   for(auto it = sourcePropertyTree.begin(); it!=sourcePropertyTree.end();
       it++)
   {
      destPropertyTree.push_back(PropertyTree::value_type(it->first,
                                 PropertyTree(StateElement(it->first, it->second.data()))));
   }

   return destPropertyTree;
}
}
