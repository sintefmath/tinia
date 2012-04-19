#include "jobobserver/Job.hpp"
namespace jobobserver {

Job::Job()
   : m_policyLib(new policylib::PolicyLib)
{
}

}

std::shared_ptr<policylib::PolicyLib> jobobserver::Job::getPolicylib()
{
   return m_policyLib;
}

bool jobobserver::Job::init()
{
   return true;
}

void jobobserver::Job::cleanup()
{
}

bool jobobserver::Job::periodic()
{
   return true;
}

void jobobserver::Job::quit()
{
	cleanup();
	m_policyLib->releaseAllListeners();
}
