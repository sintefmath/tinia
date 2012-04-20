#include <boost/test/unit_test.hpp>
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/PolicyLock.hpp"
#include "tinia/policy/StateListener.hpp"
#include <memory>
using namespace tinia;
class PolicyListenerFixture : policy::StateListener
{
public:
   PolicyListenerFixture() : policy(new policy::Policy()),
      hasSeenEvent(false)
   {
      policy->addStateListener(this);
   }

   ~PolicyListenerFixture() { policy->removeStateListener(this); }

   void stateElementModified(policy::StateElement *stateElement)
   {
      hasSeenEvent = true;
   }

   std::shared_ptr<policy::Policy> policy;
   bool hasSeenEvent;

};

BOOST_FIXTURE_TEST_CASE(SimpleLockTest, PolicyListenerFixture)
{
   {// Scoped lock
      BOOST_CHECK(!hasSeenEvent);
      policy::PolicyLock policyLock(policy);
      BOOST_CHECK(!hasSeenEvent);
      policy->addElement("AValueKey", "AValue");
      BOOST_CHECK(!hasSeenEvent);
      policy->updateElement("AValueKey", "ANewValue");
      BOOST_CHECK(!hasSeenEvent);
   }
   BOOST_CHECK(hasSeenEvent);

}

