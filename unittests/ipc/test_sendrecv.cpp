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

BOOST_AUTO_TEST_SUITE( IpcMsgSendRecv )


struct SendRecvFixture : public SendRecvFixtureBase
{
    size_t              m_client_bytes_to_send;
    size_t              m_client_bytes_received;
    size_t              m_server_bytes_to_send;
    size_t              m_server_bytes_received;


    void
    run()
    {
        fprintf( stderr, "Test sendrecv.\n" );
        m_client_bytes_received = 0;
        m_server_bytes_received = 0;
        SendRecvFixtureBase::run();
    }
    
    int
    serverConsumer( const char* buffer,
                    const size_t buffer_bytes,
                    const int part,
                    
                    const int more )
    {
        ScopeTrace scope_trace( this, __func__ );
        if( part == 0 ) {
            // first
        }
        {
            Locker locker( this->server_lock );
            m_server_bytes_received += buffer_bytes;
        }
        if( more == 0 ) {
            // last
        }
        return 0;
       
    }

    int
    serverProducer( int* more,
                    char* buffer,
                    size_t* buffer_bytes,
                    const size_t buffer_size,
                    const int part )
    {
        ScopeTrace scope_trace( this, __func__ );
        size_t bytes_to_send;
        {
            Locker Locker( this->server_lock );
            bytes_to_send = m_server_bytes_to_send;
        }
        
        size_t sent = part*buffer_size;
        if( sent < bytes_to_send ) {
            if( sent + buffer_size < bytes_to_send ) {
                *buffer_bytes = buffer_size;
                *more = 1;
            }
            else {
                *buffer_bytes = bytes_to_send - sent;
                *more = 0;
            }
        }
        else {
            *buffer_bytes = 0;
            *more = 0;
        }
        return 0;   
    }
  
    int
    clientProducer( int* more,
                    char* buffer,
                    size_t* buffer_bytes,
                    const size_t buffer_size,
                    const int part )
    {
        ScopeTrace scope_trace( this, __func__ );
        size_t bytes_to_send;
        {
            Locker locker( this->client_lock );
            bytes_to_send = m_client_bytes_to_send;
        }
        
        size_t sent = part*buffer_size;
        if( sent < bytes_to_send ) {
            if( sent + buffer_size < bytes_to_send ) {
                *buffer_bytes = buffer_size;
                *more = 1;
            }
            else {
                *buffer_bytes = bytes_to_send - sent;
                *more = 0;
            }
        }
        else {
            *buffer_bytes = 0;
            *more = 0;
        }
        return 0;   
    }

    int
    clientConsumer( const char* buffer,
                    const size_t buffer_bytes,
                    const int part,
                    const int more ) 
    {
        ScopeTrace scope_trace( this, __func__ );
        if( part == 0 ) {
            // first
        }
        {
            Locker locker( this->client_lock );
            m_client_bytes_received += buffer_bytes;
        }
        if( more == 0 ) {
            // last
        }
        return 0;
    }
        
    
};



BOOST_FIXTURE_TEST_CASE( send, SendRecvFixture )
{
    ipc_msg_fake_shmem = 1;
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    m_clients = 3;
    m_failure_is_an_option = 0;
    m_client_bytes_to_send = 10000;
    m_server_bytes_to_send = 0;
    run();
    BOOST_REQUIRE_EQUAL( m_client_bytes_received, m_clients*m_server_bytes_to_send );
    BOOST_REQUIRE_EQUAL( m_server_bytes_received, m_clients*m_client_bytes_to_send );
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
}

BOOST_FIXTURE_TEST_CASE( recv, SendRecvFixture )
{
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    m_clients = 3;
    m_failure_is_an_option = 0;
    m_client_bytes_to_send = 0;
    m_server_bytes_to_send = 7000;
    run();
    BOOST_REQUIRE_EQUAL( m_client_bytes_received, m_clients*m_server_bytes_to_send );
    BOOST_REQUIRE_EQUAL( m_server_bytes_received, m_clients*m_client_bytes_to_send );
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
}

BOOST_AUTO_TEST_SUITE_END()
