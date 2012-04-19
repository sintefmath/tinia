#ifndef STATESCHEMALISTENERHANDLER_HPP
#define STATESCHEMALISTENERHANDLER_HPP
#include <list>
#include <map>
#include <vector>

#include "policylib/StateSchemaListener.hpp"

namespace policylib {
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
#endif // STATESCHEMALISTENERHANDLER_HPP
