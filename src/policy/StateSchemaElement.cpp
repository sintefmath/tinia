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
