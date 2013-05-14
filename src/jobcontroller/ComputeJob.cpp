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

#include "tinia/jobcontroller/ComputeJob.hpp"
#include "tinia/model/ExposedModel.hpp"
#include <stdexcept>
namespace tinia {
namespace jobcontroller
{

ComputeJob::ComputeJob(  ) 
{
	m_model->addElement("status", "initiating");
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
		m_model->updateElement("status", "finished");
	}
	catch (Interrupted &e)
	{
		m_model->updateElement("status", "terminated");
	}
	catch (std::exception &e)
	{
		std::string status = "failed: ";
		status.append(e.what());
		m_model->updateElement("status", status);
	}
	catch ( ... )
	{
		m_model->updateElement("status", "failed");
	}
}

void
ComputeJob::start()
{
  assert(!m_computeThread || !m_computeThread->joinable());
  m_computeThread.reset(new boost::thread(boost::ref(*this)));
  m_model->updateElement("status", "running");
}

bool
ComputeJob::isRunning()
{
	if (!m_computeThread->joinable())
	{
		return false;
	}
	std::string status;
	m_model->getElementValue("status", status);
	return status == "running";
}

void 
ComputeJob::cleanup()
{
  m_model->updateElement("status", "terminating");
  if (!m_computeThread || m_computeThread->joinable())
  {
    m_computeThread->join();
  }
  m_model->updateElement("status", "terminated");
}

 void
 ComputeJob::throwOnTerminating()
 {
	std::string status;
	m_model->getElementValue("status", status);
	if (status == "terminating")
	{
		throw Interrupted();

//		throw std::runtime_error("Whaat");

	}
}



}
}
