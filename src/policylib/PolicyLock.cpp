#include "tinia/policylib/PolicyLock.hpp"

namespace policylib {

PolicyLock::PolicyLock(std::shared_ptr<PolicyLib>& policylib)
   : m_scoped_lock(policylib->getPolicyMutex()),
     m_policylib(policylib)
{
   policylib->holdStateEvents();
}

PolicyLock::~PolicyLock()
{
   m_scoped_lock.unlock();
   m_policylib->releaseStateEvents();
}

} // namespace policylib
