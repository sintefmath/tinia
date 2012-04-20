#include "tinia/jobobserver/ComputeJob.hpp"
#include <boost/test/unit_test.hpp>

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
	std::shared_ptr<policylib::PolicyLib> policy=job.getPolicylib();
	
	job.init();
	job.start();
	job.quit();
	std::string status;
	policy->getElementValue("status", status);
	BOOST_CHECK_EQUAL(status, "terminated");
}

BOOST_AUTO_TEST_CASE( throwingJob ) {
	ThrowingComputeJob job;
	std::shared_ptr<policylib::PolicyLib> policy=job.getPolicylib();

	job.init();
	job.start();
	while (job.isRunning())
	{
		;
	}
	std::string status;
	policy->getElementValue("status", status);
	size_t colonPos;
	colonPos = status.find("failed:");
        BOOST_CHECK_EQUAL(colonPos, 0u);
}

BOOST_AUTO_TEST_CASE( exitingJob ) {
	CompletingComputeJob job;
	std::shared_ptr<policylib::PolicyLib> policy=job.getPolicylib();

	job.init();
	job.start();
	while (job.isRunning())
	{
		;
	}
	std::string status;
	policy->getElementValue("status", status);
	BOOST_CHECK_EQUAL(status, "finished");
}

BOOST_AUTO_TEST_SUITE_END()
