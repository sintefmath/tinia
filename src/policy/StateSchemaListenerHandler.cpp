#include "tinia/policy/StateSchemaListenerHandler.hpp"

namespace tinia {
policy::StateSchemaListenerHandler::StateSchemaListenerHandler()
{
}

void policy::StateSchemaListenerHandler::addStateSchemaListener(
      policy::StateSchemaListener *listener)
{
   m_listeners.push_back(listener);
}

void policy::StateSchemaListenerHandler::removeStateSchemaListener(
      policy::StateSchemaListener *listener)
{
   m_listeners.remove(listener);
}

void policy::StateSchemaListenerHandler::addStateSchemaListener(
      std::string key, policy::StateSchemaListener *listener)
{
   m_keyListeners[key].push_back(listener);
}

void policy::StateSchemaListenerHandler::removeStateSchemaListener(
      std::string key, policy::StateSchemaListener *listener)
{
   m_keyListeners[key].remove(listener);
   if(m_keyListeners[key].size() ==0)
   {
      m_keyListeners.erase(key);
   }
}

void policy::StateSchemaListenerHandler::fireStateSchemaElementAdded(
      policy::StateSchemaElement *element)
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

void policy::StateSchemaListenerHandler::fireStateSchemaElementRemoved(
      policy::StateSchemaElement *element)
{
   for(auto it = m_listeners.begin(); it != m_listeners.end(); it++)
   {
      (*it)->stateSchemaElementRemoved(element);
   }
}

void policy::StateSchemaListenerHandler::fireStateSchemaElementModified(
      policy::StateSchemaElement *element)
{
   for(auto it = m_listeners.begin(); it != m_listeners.end(); it++)
   {
      (*it)->stateSchemaElementModified(element);
   }
}
}
