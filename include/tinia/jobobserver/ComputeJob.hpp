#pragma once
#include <string>
//#include <mutex> 
#include "tinia/jobobserver/Job.hpp"
#include <boost/thread/thread.hpp>

namespace jobobserver
{

  class ComputeJob : public Job
{
public:
	class Interrupted: public std::exception
	{
		virtual const char* what() const throw()
		{
			return "jobobserver::ComputeJob interrupted";
		}
	};

    ComputeJob( );

    virtual ~ComputeJob( );

    void
    start();

	bool
	isRunning();

  // functor for threading must be public
  void 
  operator()(); 


protected:
	virtual void cleanup();

    virtual
    void
    run() = 0;

	void throwOnTerminating();


private:
  boost::thread m_computeThread;

};




} // of namespace Trell
