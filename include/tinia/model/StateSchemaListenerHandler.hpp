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

#ifndef STATESCHEMALISTENERHANDLER_HPP
#define STATESCHEMALISTENERHANDLER_HPP
#include <list>
#include <map>
#include <vector>

#include "tinia/policy/StateSchemaListener.hpp"

namespace tinia {
namespace policy {
class StateSchemaListenerHandler
{
public:
   StateSchemaListenerHandler();
   void addStateSchemaListener(StateSchemaListener *listener);
   void removeStateSchemaListener(StateSchemaListener *listener);
   void addStateSchemaListener(std::string key, StateSchemaListener *listener);
   void removeStateSchemaListener(std::string key, StateSchemaListener *listener);

   void fireStateSchemaElementAdded(StateSchemaElement *element);
   void fireStateSchemaElementRemoved(StateSchemaElement *element);
   void fireStateSchemaElementModified(StateSchemaElement *element);

private:
   std::list<StateSchemaListener*> m_listeners;
   std::map<std::string, std::list<StateSchemaListener*> > m_keyListeners;
};
}
}
#endif // STATESCHEMALISTENERHANDLER_HPP
