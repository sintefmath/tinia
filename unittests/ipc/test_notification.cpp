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
#include "test_fixture.hpp"

BOOST_AUTO_TEST_SUITE( IpcMsgNotification )

struct NotificationFixture
    : public SendRecvFixtureBase
{
    int             m_flag;
    int             m_longpolling_clients;
    pthread_cond_t  m_longpolling_clients_cond;
    int             m_clients_that_got_flag;

    NotificationFixture()
        : m_flag(0),
          m_longpolling_clients(0),
          m_longpolling_clients_cond( PTHREAD_COND_INITIALIZER ),
          m_clients_that_got_flag( 0 )
    {}
    
    
    void
    run()
    {
        m_clients_should_longpoll = 1;
        SendRecvFixtureBase::run();
    }

    int
    inner()
    {
        struct timespec timeout;
        clock_gettime( CLOCK_REALTIME, &timeout );
        timeout.tv_sec += 1;

        int ret=0;
        while( m_longpolling_clients != m_clients ) {
            int rc = pthread_cond_timedwait( &m_longpolling_clients_cond,
                                             &lock,
                                             &timeout );
            if( rc == ETIMEDOUT ) {
                ret = -1;
                BOOST_CHECK( false && "timed out while waiting for clients to start longpolling" );
            }
        }
        m_flag = 1;
        
        assert( pthread_mutex_unlock( &lock ) == 0 );

        int rc = ipc_msg_server_notify( m_server );
        
        assert( pthread_mutex_lock( &lock ) == 0 );
        BOOST_CHECK_EQUAL( rc, 0 );
        return ret;
    }
    
    
    int
    serverConsumer( const char* buffer,
                    const size_t buffer_bytes,
                    const int part,
                    const int more )
    {
        return 0;
    }
    
    int
    serverProducer( int* more,
                    char* buffer,
                    size_t* buffer_bytes,
                    const size_t buffer_size,
                    const int part )
    {
        assert( pthread_mutex_lock( &lock ) == 0 );
        int flag = m_flag;
        assert( pthread_mutex_unlock( &lock ) == 0  );
        *((int*)buffer) = flag;
        *buffer_bytes = sizeof(flag);
        *more = 0;
        return 0;   
    }

    int
    clientProducer( int* more,
                    char* buffer,
                    size_t* buffer_bytes,
                    const size_t buffer_size,
                    const int part )
    {
        *buffer_bytes = 0;
        *more = 0;
        return 0;   
    }
    
    int
    clientConsumer( const char* buffer,
                    const size_t buffer_bytes,
                    const int part,
                    const int more ) 
    {
        if( *((int*)buffer) == 0 ) {
            assert( pthread_mutex_lock( &lock ) == 0 );
            m_longpolling_clients++;
            if( m_longpolling_clients == m_clients ) {
                assert( pthread_cond_signal( &m_longpolling_clients_cond ) == 0  );   
            }
            assert( pthread_mutex_unlock( &lock ) == 0  );
            return 1;
        }
        else {
            assert( pthread_mutex_lock( &lock ) == 0 );
            m_clients_that_got_flag++;
            assert( pthread_mutex_unlock( &lock ) == 0  );
        }
        return 0;
    }                                        
    
};

BOOST_FIXTURE_TEST_CASE( wait, NotificationFixture )
{
    ipc_msg_fake_shmem = 1;
    m_clients = 3;
    m_failure_is_an_option = 0;
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    run();
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );    
    BOOST_REQUIRE_EQUAL( m_clients_that_got_flag, m_clients );
}


BOOST_AUTO_TEST_SUITE_END()
