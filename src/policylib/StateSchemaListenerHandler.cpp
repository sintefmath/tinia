#include "tinia/policylib/StateSchemaListenerHandler.hpp"


policylib::StateSchemaListenerHandler::StateSchemaListenerHandler()
{
}

void policylib::StateSchemaListenerHandler::addStateSchemaListener(
      policylib::StateSchemaListener *listener)
{
   m_listeners.push_back(listener);
}

void policylib::StateSchemaListenerHandler::removeStateSchemaListener(
      policylib::StateSchemaListener *listener)
{
   m_listeners.remove(listener);
}

void policylib::StateSchemaListenerHandler::addStateSchemaListener(
      std::string key, policylib::StateSchemaListener *listener)
{
   m_keyListeners[key].push_back(listener);
}

void policylib::StateSchemaListenerHandler::removeStateSchemaListener(
      std::string key, policylib::StateSchemaListener *listener)
{
   m_keyListeners[key].remove(listener);
   if(m_keyListeners[key].size() ==0)
   {
      m_keyListeners.erase(key);
   }
}

void policylib::StateSchemaListenerHandler::fireStateSchemaElementAdded(
      policylib::StateSchemaElement *element)
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

void policylib::StateSchemaListenerHandler::fireStateSchemaElementRemoved(
      policylib::StateSchemaElement *element)
{
   for(auto it = m_listeners.begin(); it != m_listeners.end(); it++)
   {
      (*it)->stateSchemaElementRemoved(element);
   }
}

void policylib::StateSchemaListenerHandler::fireStateSchemaElementModified(
      policylib::StateSchemaElement *element)
{
   for(auto it = m_listeners.begin(); it != m_listeners.end(); it++)
   {
      (*it)->stateSchemaElementModified(element);
   }
}
