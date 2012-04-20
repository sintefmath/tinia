#include "tinia/policy/StateListenerHandler.hpp"

namespace tinia {
void policy::StateListenerHandler::addStateListener(policy::StateListener *listener)
{
   m_listeners.push_back(listener);
}
void policy::StateListenerHandler::addStateListener(std::string key, policy::StateListener *listener)
{
   m_keylisteners[key].push_back(listener);
}

void policy::StateListenerHandler::removeStateListener(std::string key, policy::StateListener *listener)
{
   m_keylisteners[key].remove(listener);
   if(m_keylisteners[key].size() == 0)
   {
      m_keylisteners.erase(key);
   }
}

void policy::StateListenerHandler::removeStateListener(policy::StateListener *listener)
{
   m_listeners.remove(listener);
}


void policy::StateListenerHandler::fireStateElementModified(
      policy::StateElement *stateElement)
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

   std::list<policy::StateListener*> &listeners = m_keylisteners[stateElement->getKey()];
   for(auto it = listeners.begin(); it!= listeners.end(); it++)
   {
      (*it)->stateElementModified(stateElement);
   }
}

void policy::StateListenerHandler::holdEvents()
{
   m_isBuffering = true;
}

void policy::StateListenerHandler::releaseEvents()
{
   m_isBuffering = false;
   for(size_t i = 0; i < m_buffer.size(); i++ )
   {
      fireStateElementModified(&m_buffer[i]);
   }
   m_buffer.clear();
}
} // of namespace tinia
