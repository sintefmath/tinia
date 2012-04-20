#include "tinia/policylib/StateListenerHandler.hpp"

void policylib::StateListenerHandler::addStateListener(policylib::StateListener *listener)
{
   m_listeners.push_back(listener);
}
void policylib::StateListenerHandler::addStateListener(std::string key, policylib::StateListener *listener)
{
   m_keylisteners[key].push_back(listener);
}

void policylib::StateListenerHandler::removeStateListener(std::string key, policylib::StateListener *listener)
{
   m_keylisteners[key].remove(listener);
   if(m_keylisteners[key].size() == 0)
   {
      m_keylisteners.erase(key);
   }
}

void policylib::StateListenerHandler::removeStateListener(policylib::StateListener *listener)
{
   m_listeners.remove(listener);
}


void policylib::StateListenerHandler::fireStateElementModified(
      policylib::StateElement *stateElement)
{
   if(m_isBuffering)
   {
      m_buffer.push_back(StateElement(*stateElement));
      return;
   }
   for(auto it = m_listeners.begin(); it!= m_listeners.end(); it++)
   {
      (*it)->stateElementModified(stateElement);
   }

   std::list<policylib::StateListener*> &listeners = m_keylisteners[stateElement->getKey()];
   for(auto it = listeners.begin(); it!= listeners.end(); it++)
   {
      (*it)->stateElementModified(stateElement);
   }
}

void policylib::StateListenerHandler::holdEvents()
{
   m_isBuffering = true;
}

void policylib::StateListenerHandler::releaseEvents()
{
   m_isBuffering = false;
   for(size_t i = 0; i < m_buffer.size(); i++ )
   {
      fireStateElementModified(&m_buffer[i]);
   }
   m_buffer.clear();
}
