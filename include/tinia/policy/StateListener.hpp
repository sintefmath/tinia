#ifndef STATELISTENER_HPP
#define STATELISTENER_HPP
#include "tinia/policy/StateElement.hpp"
#include <memory>


namespace tinia {
namespace policy {
/**
  The basic listener for any changes in the State of the policy. I.e. this
  listener is notified whenever specific properties are changed.
  */
class StateListener
{
public:

   /**
     This is called whenever a specific element is changed.
     \param stateElement pointer to the StateElement being changed. The sender is
     responsible for the memory of this pointer
     */
   virtual void stateElementModified(policy::StateElement * stateElement) = 0;
};
}
}
#endif // STATELISTENER_HPP
