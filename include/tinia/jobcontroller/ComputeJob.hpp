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

#pragma once
#include <string>
//#include <mutex> 
#include "tinia/jobcontroller/Job.hpp"
#include <boost/thread/thread.hpp>

namespace tinia {
namespace jobcontroller
{

  class ComputeJob : public Job
{
public:
	class Interrupted: public std::exception
	{
		virtual const char* what() const throw()
		{
			return "jobcontroller::ComputeJob interrupted";
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



}
}
