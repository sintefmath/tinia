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

#include "tinia/model/impl/StateListenerHandler.hpp"

namespace tinia {
void model::impl::StateListenerHandler::addStateListener(model::StateListener *listener)
{
   m_listeners.push_back(listener);
}
void model::impl::StateListenerHandler::addStateListener(std::string key, model::StateListener *listener)
{
   m_keylisteners[key].push_back(listener);
}

void model::impl::StateListenerHandler::removeStateListener(std::string key, model::StateListener *listener)
{
   m_keylisteners[key].remove(listener);
   if(m_keylisteners[key].size() == 0)
   {
      m_keylisteners.erase(key);
   }
}

void model::impl::StateListenerHandler::removeStateListener(model::StateListener *listener)
{
   m_listeners.remove(listener);
}


void model::impl::StateListenerHandler::fireStateElementModified(
      model::StateElement *stateElement)
{
   if(m_isBuffering)
   {
      m_buffer.push_back(StateElement(*stateElement));
      return;
   }
   for(std::list<model::StateListener*>::iterator it = m_listeners.begin(); it!= m_listeners.end(); it++)
   {
      (*it)->stateElementModified(stateElement);
   }

   std::list<model::StateListener*> &listeners = m_keylisteners[stateElement->getKey()];
   for(std::list<model::StateListener*>::iterator it = listeners.begin(); it!= listeners.end(); it++)
   {
      (*it)->stateElementModified(stateElement);
   }
}

void model::impl::StateListenerHandler::holdEvents()
{
   m_isBuffering = true;
}

void model::impl::StateListenerHandler::releaseEvents()
{
   m_isBuffering = false;
   for(size_t i = 0; i < m_buffer.size(); i++ )
   {
      fireStateElementModified(&m_buffer[i]);
   }
   m_buffer.clear();
}

} // of namespace tinia
