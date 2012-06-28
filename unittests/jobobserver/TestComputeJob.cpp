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
#include <boost/test/unit_test.hpp>

using namespace tinia;

class TestComputeJob1 : public jobobserver::ComputeJob
{
public:
	virtual ~TestComputeJob1() 
	{
	}
	virtual void run()
	{
		while (1)
		{
			throwOnTerminating();
		}
	}
};

class ThrowingComputeJob : public jobobserver::ComputeJob
{
public:
	virtual ~ThrowingComputeJob() 
	{
	}
	virtual void run()
	{
		throw std::runtime_error("I failed");
	}
};

class CompletingComputeJob : public jobobserver::ComputeJob
{
public:
	virtual ~CompletingComputeJob() 
	{
	}
	virtual void run()
	{
	}
};


BOOST_AUTO_TEST_SUITE( ComputeJob )

BOOST_AUTO_TEST_CASE( startAndStop ) {
	TestComputeJob1 job;
	job.init();
	job.start();
	job.quit();
}

BOOST_AUTO_TEST_CASE( terminatingJob ) {
	TestComputeJob1 job;
	std::shared_ptr<model::ExposedModel> model=job.getExposedModel();
	
	job.init();
	job.start();
	job.quit();
	std::string status;
	model->getElementValue("status", status);
	BOOST_CHECK_EQUAL(status, "terminated");
}

BOOST_AUTO_TEST_CASE( throwingJob ) {
	ThrowingComputeJob job;
	std::shared_ptr<model::ExposedModel> model=job.getExposedModel();

	job.init();
	job.start();
	while (job.isRunning())
	{
		;
	}
	std::string status;
	model->getElementValue("status", status);
	size_t colonPos;
	colonPos = status.find("failed:");
        BOOST_CHECK_EQUAL(colonPos, 0u);
}

BOOST_AUTO_TEST_CASE( exitingJob ) {
	CompletingComputeJob job;
	std::shared_ptr<model::ExposedModel> model=job.getExposedModel();

	job.init();
	job.start();
	while (job.isRunning())
	{
		;
	}
	std::string status;
	model->getElementValue("status", status);
	BOOST_CHECK_EQUAL(status, "finished");
}

BOOST_AUTO_TEST_SUITE_END()
