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

#include "tinia/model/StateElement.hpp"
namespace tinia {
model::StateElement::StateElement(std::string name, const impl::ElementData &data)
   : m_name(name.c_str()), m_data(data)
{

}

model::StateElement::StateElement() {}

std::string model::StateElement::getKey() const
{
   return m_name;
}


std::string model::StateElement::getXSDType() const
{
   return m_data.getXSDType();
}

std::string model::StateElement::getStringValue() const
{
   return m_data.getStringValue();
}

model::StateElement::PropertyTree model::StateElement::getPropertyTree() const
{
   // TODO Make this more effective by implementing a "build on demand"-tree?
   // TODO Make this function return by reference (i.e. make destPropertyTree
   // an object member)?
   PropertyTree destPropertyTree;

   const impl::ElementData::PropertyTree& sourcePropertyTree  = m_data.getPropertyTree();
   for(auto it = sourcePropertyTree.begin(); it!=sourcePropertyTree.end();
       it++)
   {
      destPropertyTree.push_back(PropertyTree::value_type(it->first,
                                 PropertyTree(StateElement(it->first, it->second.data()))));
   }

   return destPropertyTree;
}
}
