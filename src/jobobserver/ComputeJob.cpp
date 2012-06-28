/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

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
