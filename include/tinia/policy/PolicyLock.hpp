#ifndef POLICY_POLICYLOCK_HPP
#define POLICY_POLICYLOCK_HPP
#include "boost/thread.hpp"
#include "tinia/policy/Policy.hpp"
#include <memory>

namespace tinia {
namespace policy {
class PolicyLock
{
public:
   PolicyLock(std::shared_ptr<Policy>& policy);
   ~PolicyLock();


private:
   Policy::scoped_lock m_scoped_lock;
   std::shared_ptr<Policy> m_policy;
};

} // namespace policy
} // namespace tinia
#endif // POLICY_POLICYLOCK_HPP
