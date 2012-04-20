#include "tinia/jobobserver/ComputeJob.hpp"
#include "tinia/policy/Policy.hpp"

namespace tinia {
namespace jobobserver
{

ComputeJob::ComputeJob(  ) 
{
	m_policy->addElement("status", "initiating");
}


ComputeJob::~ComputeJob( )
{
  quit();
}


void
ComputeJob::operator()()
{
	try
	{
		run();
		m_policy->updateElement("status", "finished");
	}
	catch (Interrupted &e)
	{
		m_policy->updateElement("status", "terminated");
	}
	catch (std::exception &e)
	{
		std::string status = "failed: ";
		status.append(e.what());
		m_policy->updateElement("status", status);
	}
	catch ( ... )
	{
		m_policy->updateElement("status", "failed");
	}
}

void
ComputeJob::start()
{
  assert(!m_computeThread.joinable());
  boost::thread t( boost::ref( *this ) );
  m_computeThread = std::move( t );
  m_policy->updateElement("status", "running");
}

bool
ComputeJob::isRunning()
{
	if (!m_computeThread.joinable())
	{
		return false;
	}
	std::string status;
	m_policy->getElementValue("status", status);
	return status == "running";
}

void 
ComputeJob::cleanup()
{
  m_policy->updateElement("status", "terminating");
  if (m_computeThread.joinable())
  {
    m_computeThread.join();
  }
  m_policy->updateElement("status", "terminated");
}

 void
 ComputeJob::throwOnTerminating()
 {
	std::string status;
	m_policy->getElementValue("status", status);
	if (status == "terminating")
	{
		throw Interrupted();

//		throw std::runtime_error("Whaat");

	}
}



}
}
