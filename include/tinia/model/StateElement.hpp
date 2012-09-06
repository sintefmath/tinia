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
#include "tinia/model/impl/ElementDataFactory.hpp"
#include <boost/shared_ptr.hpp>
#include <string>

namespace tinia {
namespace model {
class StateElement
{
public:
  typedef impl::ElementData::PropertyTree PropertyTree;
   StateElement();
   StateElement(std::string name, const impl::ElementData& data);

   std::string getKey() const;
   std::string getXSDType() const;
   template<typename T>
   void getValue(T &t) const;

   std::string getStringValue() const;

   PropertyTree getPropertyTree() const;

private:
   std::string m_name;
   impl::ElementData m_data;
   impl::ElementDataFactory m_factory;
};
template<typename T>
void StateElement::getValue(T& t) const
{
   // TODO: Type checking
   m_factory.createT( m_data, t );
}
}
}

