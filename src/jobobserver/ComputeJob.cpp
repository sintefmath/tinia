#include "tinia/jobobserver/ComputeJob.hpp"
#include "tinia/policylib/PolicyLib.hpp"

namespace tinia {
namespace jobobserver
{

ComputeJob::ComputeJob(  ) 
{
	m_policyLib->addElement("status", "initiating");
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
		m_policyLib->updateElement("status", "finished");
	}
	catch (Interrupted &e)
	{
		m_policyLib->updateElement("status", "terminated");
	}
	catch (std::exception &e)
	{
		std::string status = "failed: ";
		status.append(e.what());
		m_policyLib->updateElement("status", status);
	}
	catch ( ... )
	{
		m_policyLib->updateElement("status", "failed");
	}
}

void
ComputeJob::start()
{
  assert(!m_computeThread.joinable());
  boost::thread t( boost::ref( *this ) );
  m_computeThread = std::move( t );
  m_policyLib->updateElement("status", "running");
}

bool
ComputeJob::isRunning()
{
	if (!m_computeThread.joinable())
	{
		return false;
	}
	std::string status;
	m_policyLib->getElementValue("status", status);
	return status == "running";
}

void 
ComputeJob::cleanup()
{
  m_policyLib->updateElement("status", "terminating");
  if (m_computeThread.joinable())
  {
    m_computeThread.join();
  }
  m_policyLib->updateElement("status", "terminated");
}

 void
 ComputeJob::throwOnTerminating()
 {
	std::string status;
	m_policyLib->getElementValue("status", status);
	if (status == "terminating")
	{
		throw Interrupted();

//		throw std::runtime_error("Whaat");

	}
}



}
}
