#ifndef STATESCHEMALISTENER_HPP
#define STATESCHEMALISTENER_HPP
#include "tinia/policy/StateSchemaElement.hpp"

namespace tinia {
namespace policy {
class StateSchemaListener
{
public:
   virtual void stateSchemaElementAdded(policy::StateSchemaElement*
                                        stateSchemaElement) = 0;
   virtual void stateSchemaElementRemoved(policy::StateSchemaElement*
                                          stateSchemaElement) = 0;
   virtual void stateSchemaElementModified(policy::StateSchemaElement*
                                           stateSchemaElement) = 0;
};
}
}
#endif // STATESCHEMALISTENER_HPP
