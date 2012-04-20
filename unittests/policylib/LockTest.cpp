#include <boost/test/unit_test.hpp>
#include "tinia/policylib/PolicyLib.hpp"
#include "tinia/policylib/PolicyLock.hpp"
#include "tinia/policylib/StateListener.hpp"
#include <memory>

class PolicyLibListenerFixture : policylib::StateListener
{
public:
   PolicyLibListenerFixture() : policylib(new policylib::PolicyLib()),
      hasSeenEvent(false)
   {
      policylib->addStateListener(this);
   }

   ~PolicyLibListenerFixture() { policylib->removeStateListener(this); }

   void stateElementModified(policylib::StateElement *stateElement)
   {
      hasSeenEvent = true;
   }

   std::shared_ptr<policylib::PolicyLib> policylib;
   bool hasSeenEvent;

};

BOOST_FIXTURE_TEST_CASE(SimpleLockTest, PolicyLibListenerFixture)
{
   {// Scoped lock
      BOOST_CHECK(!hasSeenEvent);
      policylib::PolicyLock policyLock(policylib);
      BOOST_CHECK(!hasSeenEvent);
      policylib->addElement("AValueKey", "AValue");
      BOOST_CHECK(!hasSeenEvent);
      policylib->updateElement("AValueKey", "ANewValue");
      BOOST_CHECK(!hasSeenEvent);
   }
   BOOST_CHECK(hasSeenEvent);

}

