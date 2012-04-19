#ifndef POLICYLIB_POLICYLOCK_HPP
#define POLICYLIB_POLICYLOCK_HPP
#include "boost/thread.hpp"
#include "policylib/PolicyLib.hpp"
#include <memory>

namespace policylib {
class PolicyLock
{
public:
   PolicyLock(std::shared_ptr<PolicyLib>& policyLib);
   ~PolicyLock();


private:
   PolicyLib::scoped_lock m_scoped_lock;
   std::shared_ptr<PolicyLib> m_policylib;
};

} // namespace policylib

#endif // POLICYLIB_POLICYLOCK_HPP
