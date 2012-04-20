#include "tinia/policylib/StateElement.hpp"
namespace tinia {
policylib::StateElement::StateElement(std::string name, const ElementData &data)
   : m_name(name.c_str()), m_data(data)
{

}

policylib::StateElement::StateElement() {}

std::string policylib::StateElement::getKey() const
{
   return m_name;
}


std::string policylib::StateElement::getXSDType() const
{
   return m_data.getXSDType();
}

std::string policylib::StateElement::getStringValue() const
{
   return m_data.getStringValue();
}

policylib::StateElement::PropertyTree policylib::StateElement::getPropertyTree() const
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
