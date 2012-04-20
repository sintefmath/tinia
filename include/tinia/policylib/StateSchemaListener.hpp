#ifndef STATESCHEMALISTENER_HPP
#define STATESCHEMALISTENER_HPP
#include "tinia/policylib/StateSchemaElement.hpp"

namespace tinia {
namespace policylib {
class StateSchemaListener
{
public:
   virtual void stateSchemaElementAdded(policylib::StateSchemaElement*
                                        stateSchemaElement) = 0;
   virtual void stateSchemaElementRemoved(policylib::StateSchemaElement*
                                          stateSchemaElement) = 0;
   virtual void stateSchemaElementModified(policylib::StateSchemaElement*
                                           stateSchemaElement) = 0;
};
}
}
#endif // STATESCHEMALISTENER_HPP
