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

#include "tinia/model/impl/StateSchemaListenerHandler.hpp"

namespace tinia {
model::impl::StateSchemaListenerHandler::StateSchemaListenerHandler()
{
}

void model::impl::StateSchemaListenerHandler::addStateSchemaListener(
      model::StateSchemaListener *listener)
{
   m_listeners.push_back(listener);
}

void model::impl::StateSchemaListenerHandler::removeStateSchemaListener(
      model::StateSchemaListener *listener)
{
   m_listeners.remove(listener);
}

void model::impl::StateSchemaListenerHandler::addStateSchemaListener(
      std::string key, model::StateSchemaListener *listener)
{
   m_keyListeners[key].push_back(listener);
}

void model::impl::StateSchemaListenerHandler::removeStateSchemaListener(
      std::string key, model::StateSchemaListener *listener)
{
   m_keyListeners[key].remove(listener);
   if(m_keyListeners[key].size() ==0)
   {
      m_keyListeners.erase(key);
   }
}

void model::impl::StateSchemaListenerHandler::fireStateSchemaElementAdded(
      model::StateSchemaElement *element)
{

   for(auto it = m_listeners.begin(); it != m_listeners.end(); it++)
   {
      (*it)->stateSchemaElementAdded(element);
   }

   std::string key = element->getKey();
   if(m_keyListeners.find(key)!=m_keyListeners.end())
   {
      auto listeners = m_keyListeners[key];
      for(auto it = listeners.begin(); it != listeners.end(); it++)
      {
         (*it)->stateSchemaElementAdded(element);
      }
   }
}

void model::impl::StateSchemaListenerHandler::fireStateSchemaElementRemoved(
      model::StateSchemaElement *element)
{
   for(auto it = m_listeners.begin(); it != m_listeners.end(); it++)
   {
      (*it)->stateSchemaElementRemoved(element);
   }

   std::string key = element->getKey();
   if(m_keyListeners.find(key)!=m_keyListeners.end())
   {
      auto listeners = m_keyListeners[key];
      for(auto it = listeners.begin(); it != listeners.end(); it++)
      {
         (*it)->stateSchemaElementRemoved(element);
      }
   }
}

void model::impl::StateSchemaListenerHandler::fireStateSchemaElementModified(
      model::StateSchemaElement *element)
{
   for(auto it = m_listeners.begin(); it != m_listeners.end(); it++)
   {
      (*it)->stateSchemaElementModified(element);
   }

   std::string key = element->getKey();
   if(m_keyListeners.find(key)!=m_keyListeners.end())
   {
      auto listeners = m_keyListeners[key];
      for(auto it = listeners.begin(); it != listeners.end(); it++)
      {
         (*it)->stateSchemaElementModified(element);
      }
   }
}
}
