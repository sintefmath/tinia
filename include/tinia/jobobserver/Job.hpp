#pragma once
#include "tinia/policy/Policy.hpp"
#include <memory>

namespace tinia {
namespace jobobserver {
class Job
{
public:
   Job( );

   virtual ~Job(  ) {};

   virtual bool init();

   virtual
   void
   cleanup();

   virtual
   bool
   periodic();

   void quit();

   virtual std::shared_ptr<policy::Policy> getPolicy();

protected:
   std::shared_ptr<policy::Policy> m_policy;
};
}
}

