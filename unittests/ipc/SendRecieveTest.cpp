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

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <csignal>                      // sigaction
#include "tinia/ipc/messenger.h"

namespace {

static
void
handle_SIGTERM(int)
{
    std::cerr << "SIGTERM\n";
    // do nothing, but break 
}

static int do_run = true;

class FixtureThread
{
public:
    FixtureThread()
    {
        struct sigaction act;
        memset( &act, 0, sizeof(act) );
        sigemptyset( &act.sa_mask );
    
        act.sa_handler = handle_SIGTERM;
        if( sigaction( SIGTERM, &act, NULL ) != 0 ) {
            std::cerr << "sigaction failed: " << strerror( errno ) << "\n";
        }
    }
    
    
    void
    operator()()
    {
/*
        std::cerr << "Enter\n";
        BOOST_CHECK( messenger_server_create( &m_endpoint, "unittest" ) == MESSENGER_OK );
        
        do {
            sleep( 1 );
        } while( do_run );
        BOOST_CHECK( messenger_server_destroy( &m_endpoint ) == MESSENGER_OK );
        std::cerr << "Exit\n";*/
    }
protected:
    messenger_server_t*    m_endpoint;
    
};



class EndpointFixture
{
public:

    
    EndpointFixture()
    {
        do_run = true;
        m_thread = boost::thread(FixtureThread());
    }

    ~EndpointFixture()
    {
        kill( getpid(), SIGTERM );
        do_run = false;
        m_thread.join();
    }

    
protected:
    boost::thread   m_thread;
    
};

BOOST_FIXTURE_TEST_CASE( createMessageBox, EndpointFixture )
{
    sleep(1);
    
}


}
