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

BOOST_AUTO_TEST_SUITE( IpcMsgErrors )


struct ErrorInduceFixture
    : public SendRecvFixtureBase
{
    enum {
        CLIENT_CONSUMER = (1<<0),
        CLIENT_PRODUCER = (1<<1),
        SERVER_CONSUMER = (1<<2),
        SERVER_PRODUCER = (1<<3)
    }               m_fail_func;
    int             m_fail_part;
    int             m_client_consumer_invocations;
    int             m_client_producer_invocations;
    int             m_server_consumer_invocations;
    int             m_server_producer_invocations;
    
    void
    run()
    {
        fprintf( stderr, "Test errors.\n" );
        ipc_msg_fake_shmem = 1;
        m_clients = 1;
        m_failure_is_an_option = 1;
        m_client_consumer_invocations = 0;
        m_client_producer_invocations = 0;
        m_server_consumer_invocations = 0;
        m_server_producer_invocations = 0;
        SendRecvFixtureBase::run();
    }
    
    int
    serverConsumer( const char* buffer,
                    const size_t buffer_bytes,
                    const int part,
                    const int more )
    {
        Locker locker( this->server_lock );
        m_server_consumer_invocations++;
        if( m_fail_func & SERVER_CONSUMER ) {
            if( part == m_fail_part ) {
                return -1;
            }
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
        int ret = 0;
        {
            Locker locker( this->server_lock );
            m_server_producer_invocations++;
            if( m_fail_func & SERVER_PRODUCER ) {
                if( part == m_fail_part ) {
                    ret = -1;
                }
            }
        }
        if( part < 2 ) {
            *more = 1;
        }
        else {
            *more = 0;
        }
        *buffer_bytes = 0;
        return ret;   
    }

    int
    clientProducer( int* more,
                    char* buffer,
                    size_t* buffer_bytes,
                    const size_t buffer_size,
                    const int part )
    {
        int ret = 0;
        {
            Locker lcoker( this->client_lock );
            m_client_producer_invocations++;
            if( m_fail_func & CLIENT_PRODUCER ) {
                if( part == m_fail_part ) {
                    ret = -1;
                }
            }
        }
        
        if( part < 2 ) {
            *more = 1;
        }
        else {
            *more = 0;
        }
        *buffer_bytes = 0;
        
        return ret;
    }
    
    int
    clientConsumer( const char* buffer,
                    const size_t buffer_bytes,
                    const int part,
                    const int more ) 
    {
        Locker locker( this->client_lock );
        m_client_consumer_invocations++;
        if( m_fail_func & CLIENT_CONSUMER ) {
            if( part == m_fail_part ) {
                return -1;
            }
        }
        return 0;
    }                                        
    
};

BOOST_FIXTURE_TEST_CASE( ClientProducerFirst, ErrorInduceFixture )
{    
    m_fail_func = CLIENT_PRODUCER;
    m_fail_part = 0;
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    run();
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    
    BOOST_REQUIRE_EQUAL( m_client_producer_invocations, 1 );
    BOOST_REQUIRE_EQUAL( m_client_consumer_invocations, 0 );
    BOOST_REQUIRE_EQUAL( m_server_consumer_invocations, 0 );
    BOOST_REQUIRE_EQUAL( m_server_producer_invocations, 0 );
}

BOOST_FIXTURE_TEST_CASE( ClientProducerMiddle, ErrorInduceFixture )
{    
    m_fail_func = CLIENT_PRODUCER;
    m_fail_part = 1;
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    run();
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    
    BOOST_REQUIRE_EQUAL( m_client_producer_invocations, 2 );
    BOOST_REQUIRE_EQUAL( m_client_consumer_invocations, 0 );
    BOOST_REQUIRE_EQUAL( m_server_consumer_invocations, 1 );
    BOOST_REQUIRE_EQUAL( m_server_producer_invocations, 0 );
}

BOOST_FIXTURE_TEST_CASE( ClientProducerLast, ErrorInduceFixture )
{    
    m_fail_func = CLIENT_PRODUCER;
    m_fail_part = 2;
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    run();
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    
    BOOST_REQUIRE_EQUAL( m_client_producer_invocations, 3 );
    BOOST_REQUIRE_EQUAL( m_client_consumer_invocations, 0 );
    BOOST_REQUIRE_EQUAL( m_server_consumer_invocations, 2 );
    BOOST_REQUIRE_EQUAL( m_server_producer_invocations, 0 );
}
#if 1
BOOST_FIXTURE_TEST_CASE( ServerConsumerFirst, ErrorInduceFixture )
{    
    m_fail_func = SERVER_CONSUMER;
    m_fail_part = 0;
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    run();
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    
    BOOST_REQUIRE_EQUAL( m_client_producer_invocations, 1 );
    BOOST_REQUIRE_EQUAL( m_client_consumer_invocations, 0 );
    BOOST_REQUIRE_EQUAL( m_server_consumer_invocations, 1 );
    BOOST_REQUIRE_EQUAL( m_server_producer_invocations, 0 );
}
BOOST_FIXTURE_TEST_CASE( ServerConsumerMiddle, ErrorInduceFixture )
{    
    m_fail_func = SERVER_CONSUMER;
    m_fail_part = 1;
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    run();
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    
    BOOST_REQUIRE_EQUAL( m_client_producer_invocations, 2 );
    BOOST_REQUIRE_EQUAL( m_client_consumer_invocations, 0 );
    BOOST_REQUIRE_EQUAL( m_server_consumer_invocations, 2 );
    BOOST_REQUIRE_EQUAL( m_server_producer_invocations, 0 );
}
BOOST_FIXTURE_TEST_CASE( ServerConsumerLast, ErrorInduceFixture )
{    
    m_fail_func = SERVER_CONSUMER;
    m_fail_part = 2;
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    run();
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    
    BOOST_REQUIRE_EQUAL( m_client_producer_invocations, 3 );
    BOOST_REQUIRE_EQUAL( m_client_consumer_invocations, 0 );
    BOOST_REQUIRE_EQUAL( m_server_consumer_invocations, 3 );
    BOOST_REQUIRE_EQUAL( m_server_producer_invocations, 0 );
}
BOOST_FIXTURE_TEST_CASE( ServerProducerFirst, ErrorInduceFixture )
{    
    m_fail_func = SERVER_PRODUCER;
    m_fail_part = 0;
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    run();
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    
    BOOST_REQUIRE_EQUAL( m_client_producer_invocations, 3 );
    BOOST_REQUIRE_EQUAL( m_client_consumer_invocations, 0 );
    BOOST_REQUIRE_EQUAL( m_server_consumer_invocations, 3 );
    BOOST_REQUIRE_EQUAL( m_server_producer_invocations, 1 );
}
BOOST_FIXTURE_TEST_CASE( ServerProducerMiddle, ErrorInduceFixture )
{    
    m_fail_func = SERVER_PRODUCER;
    m_fail_part = 1;
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    run();
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    
    BOOST_REQUIRE_EQUAL( m_client_producer_invocations, 3 );
    BOOST_REQUIRE_EQUAL( m_client_consumer_invocations, 1 );
    BOOST_REQUIRE_EQUAL( m_server_consumer_invocations, 3 );
    BOOST_REQUIRE_EQUAL( m_server_producer_invocations, 2 );
}
BOOST_FIXTURE_TEST_CASE( ServerProducerLast, ErrorInduceFixture )
{    
    m_fail_func = SERVER_PRODUCER;
    m_fail_part = 2;
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    run();
    BOOST_REQUIRE_EQUAL( ipc_msg_fake_shmem_users, 0 );
    
    BOOST_REQUIRE_EQUAL( m_client_producer_invocations, 3 );
    BOOST_REQUIRE_EQUAL( m_client_consumer_invocations, 2 );
    BOOST_REQUIRE_EQUAL( m_server_consumer_invocations, 3 );
    BOOST_REQUIRE_EQUAL( m_server_producer_invocations, 3 );
}
#endif





BOOST_AUTO_TEST_SUITE_END()
    
