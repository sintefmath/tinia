#include "tinia/jobobserver/Job.hpp"

namespace tinia {
namespace jobobserver {

Job::Job()
   : m_policy(new policy::Policy)
{
}

}

std::shared_ptr<policy::Policy> jobobserver::Job::getPolicy()
{
   return m_policy;
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
	m_policy->releaseAllListeners();
}

}
