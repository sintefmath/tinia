#ifndef JOB_HPP
#define JOB_HPP
#include "tinia/policylib/PolicyLib.hpp"
#include <memory>
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

   virtual std::shared_ptr<policylib::PolicyLib> getPolicylib();

protected:
   std::shared_ptr<policylib::PolicyLib> m_policyLib;
};
}
#endif // JOB_HPP
