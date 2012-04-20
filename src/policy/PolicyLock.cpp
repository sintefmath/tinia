#include "tinia/policy/PolicyLock.hpp"

namespace tinia {
namespace policy {

PolicyLock::PolicyLock(std::shared_ptr<Policy>& policy)
   : m_scoped_lock(policy->getPolicyMutex()),
     m_policy(policy)
{
   policy->holdStateEvents();
}

PolicyLock::~PolicyLock()
{
   m_scoped_lock.unlock();
   m_policy->releaseStateEvents();
}

} // namespace policy
} // of namespace tinia
