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
#include <cassert>
#include <ctime>
#include <cstdarg>
#include <boost/test/unit_test.hpp>
#include <tinia/ipc/ipc_msg.h>
#include "../../src/ipc/ipc_msg_internal.h"

struct SendRecvFixtureBase
{

    SendRecvFixtureBase()
        : m_server( NULL ),
          m_server_runs_flag( 0 ),
          m_failure_is_an_option( 0 ),
          m_clients( 1 ),
          m_clients_should_longpoll( 0 ),
          m_clients_initialized( 0 ),
          m_clients_exited( 0 )
    {}
    
    std::vector<pthread_t>  m_threads;
    pthread_mutex_t         lock;
    tinia_ipc_msg_server_t*               m_server;
    int                     m_server_runs_flag;
    pthread_cond_t          m_server_runs_cond;
    int                     m_failure_is_an_option;
    int                     m_clients;
    int                     m_clients_should_longpoll;
    int                     m_clients_initialized;
    pthread_cond_t          m_clients_initialized_cond;
    int                     m_clients_exited;
    pthread_cond_t          m_clients_exited_cond;
    
    
    virtual
    int
    clientProducer( int* more,
                    char* buffer,
                    size_t* buffer_bytes,
                    const size_t buffer_size,
                    const int part ) = 0;
    
    virtual
    int
    clientConsumer( const char* buffer,
                    const size_t buffer_bytes,
                    const int part,
                    const int more ) = 0;
    virtual
    int
    serverConsumer( const char* buffer,
                    const size_t buffer_bytes,
                    const int part,
                    const int more ) = 0;
    
    virtual
    int
    serverProducer( int* more,
                    char* buffer,
                    size_t* buffer_bytes,
                    const size_t buffer_size,
                    const int part ) = 0;

    virtual
    int
    inner()
    { return 0; }
    
    virtual
    void
    run()
    {
        fprintf( stderr, "test begin.\n" );
        m_clients_initialized = 0;
        m_clients_exited = 0;

        int rc;
        // Note boost::test is not thread safe. Thus, we use a lock around
        // all test macros when we go multi-threaded. Actually obtaining this
        // lock is wrappend in assert, as the lock must be held to use the
        // test macros.
        
        // test timeout
        struct timespec timeout;
        clock_gettime( CLOCK_REALTIME, &timeout );
        timeout.tv_sec += 1;

        
        pthread_mutexattr_t mutex_attr;
        
        BOOST_REQUIRE( pthread_mutexattr_init( &mutex_attr) == 0 );
        BOOST_REQUIRE( pthread_mutexattr_settype( &mutex_attr, PTHREAD_MUTEX_ERRORCHECK ) == 0 );

        
        
        BOOST_REQUIRE( pthread_mutex_init( &lock, &mutex_attr ) == 0 );

        BOOST_REQUIRE( pthread_cond_init( &m_server_runs_cond, NULL ) == 0 );
        BOOST_REQUIRE( pthread_cond_init( &m_clients_initialized_cond, NULL ) == 0 );
        BOOST_REQUIRE( pthread_cond_init( &m_clients_exited_cond, NULL ) == 0 );
        
        BOOST_REQUIRE( pthread_mutexattr_destroy( &mutex_attr) == 0 );

        // wait for server to be up and running.
        assert( pthread_mutex_lock( &lock ) == 0 );
        m_server = NULL;
        m_threads.resize( m_threads.size()+1 );

        BOOST_REQUIRE( pthread_create( &m_threads.back(),
                                       NULL,
                                       server_thread_func,
                                       this ) == 0 );
        m_server_runs_flag = 0;
        do {
            int rc = pthread_cond_timedwait( &m_server_runs_cond,
                                             &lock,
                                             &timeout );
            BOOST_REQUIRE( (rc == 0) || (rc==ETIMEDOUT) );
            if( rc == ETIMEDOUT ) {
                BOOST_CHECK( false && "timed out while waiting for server to be created."  );
                goto hung;
            }
        }
        while( m_server_runs_flag == 0 );

        BOOST_REQUIRE( m_server != NULL );
        BOOST_REQUIRE( m_server->shmem_header_ptr != NULL );
            
        // create clients
        BOOST_REQUIRE( (m_clients >= 0) && (m_clients < 10) ); // sanity check
        for(int i=0; i<m_clients; i++ ) {
            m_threads.resize( m_threads.size()+1 );
            BOOST_REQUIRE( pthread_create( &m_threads.back(),
                                           NULL,
                                           client_thread_func,
                                           this ) == 0 );
        }

        
        // wait for clients to run
        while( m_clients_initialized != m_clients ) {
            int rc = pthread_cond_timedwait( &m_clients_initialized_cond,
                                             &lock,
                                             &timeout );
            BOOST_REQUIRE( (rc == 0) || (rc==ETIMEDOUT) );
            if( rc == ETIMEDOUT ) {
                BOOST_CHECK( false && "timed out while waiting for clients to initialize."  );
                goto hung;
            }
        }
        
        if( inner() != 0 ) {
            goto hung;
        }

        // wait for clients to finish
        while( m_clients_exited != m_clients ) {
            int rc = pthread_cond_timedwait( &m_clients_exited_cond,
                                             &lock,
                                             &timeout );
            BOOST_REQUIRE( (rc == 0) || (rc==ETIMEDOUT) );
            if( rc == ETIMEDOUT ) {
                BOOST_CHECK( false && "timed out while waiting for clients to finish."  );
                goto hung;
            }
        }
        fprintf( stderr, "%s@%d: %d clients have exited.\n", __FILE__, __LINE__, m_clients_exited );
        // kill server
        BOOST_REQUIRE( m_server != NULL );
        assert( pthread_mutex_unlock( &lock ) == 0 );
        rc = ipc_msg_server_mainloop_break( m_server );
        assert( pthread_mutex_lock( &lock ) == 0 );
        BOOST_REQUIRE( rc == 0 );
        
        // wait for server thread to finsh
        while( m_server != NULL ) {
            int rc = pthread_cond_timedwait( &m_server_runs_cond,
                                             &lock,
                                             &timeout );
            BOOST_REQUIRE( (rc == 0) || (rc==ETIMEDOUT) );
            if( rc == ETIMEDOUT ) {
                BOOST_CHECK( false && "timed out while waiting for server to finish."  );
                goto hung;
            }
        }
        assert( pthread_mutex_unlock( &lock ) == 0);
        goto cleanup;
hung:
        ipc_msg_server_mainloop_break( m_server );
        sleep(1);

        // invariants:
        // - m_shared_lock locked.
        for( auto it=m_threads.begin(); it!=m_threads.end(); ++it ) {
            pthread_cancel( *it );
        }
        assert( pthread_mutex_unlock( &lock ) == 0 );

cleanup:
        for( size_t i=0; i<m_threads.size(); i++ ) {
            void* tmp;
            if( pthread_tryjoin_np( m_threads[i], &tmp ) == 0 ) {
                //std::cerr << "joined thread "<< i<< ".\n";
            }
            else {
                //std::cerr << "thread "<< i<< " is still busy.\n";
            }
        }
        //std::cerr << "done.\n";
        BOOST_REQUIRE( pthread_cond_destroy( &m_server_runs_cond ) == 0 );
        BOOST_REQUIRE( pthread_cond_destroy( &m_clients_initialized_cond ) == 0 );
        BOOST_REQUIRE( pthread_cond_destroy( &m_clients_exited_cond) == 0 );
        BOOST_REQUIRE( pthread_mutex_destroy( &lock ) == 0 );
        //ipc_msg_server_wipe( "unittest" );
        fprintf( stderr, "test end.\n" );
    }

    static
    void
    logger( void* data,
            int level,
            const char* who,
            const char* msg, ... )
    {
        SendRecvFixtureBase* that = (SendRecvFixtureBase*)data;
        assert( pthread_mutex_lock( &that->lock ) == 0 );
        char buf[1024];
        va_list args;
        va_start( args, msg );
        vsnprintf( buf, sizeof(buf), msg, args );
        va_end( args );
        switch( level ) {
        case 0: std::cerr << "[E] "; break;
        case 1: std::cerr << "[W] "; break;
        default:  std::cerr << "[I] "; break;
        }
        if( who != NULL ) {
            std::cerr << '[' << who << "]";
        }
        for( int i = strlen(who); i<40; i++ ) {
            std::cerr << ' ';
        }
        std::cerr << buf << std::endl;
        assert( pthread_mutex_unlock( &that->lock ) == 0 );
    }
    
    
    static
    int
    server_consumer( void* data,
                     const char* buffer,
                     const size_t buffer_bytes,
                     const int part,
                     const int more )
    {
        return ((SendRecvFixtureBase*)data)->serverConsumer( buffer,
                                                             buffer_bytes,
                                                             part,
                                                             more );
    }

    static
    int
    server_producer( void* data,
              int* more,
              char* buffer,
              size_t* buffer_bytes,
              const size_t buffer_size,
              const int part )
    {
        return ((SendRecvFixtureBase*)data)->serverProducer( more,
                                                             buffer,
                                                             buffer_bytes,
                                                             buffer_size,
                                                             part );
    }
    
    static
    int
    server_input_handler( ipc_msg_consumer_t* consumer_, void** consumer_data,
                          void* handler_data,
                          char* buffer,
                          size_t buffer_bytes )
    {
        *consumer_ = server_consumer;
        *consumer_data = handler_data;
        return 0;
    }

    static
    int
    server_output_handler( ipc_msg_producer_t* producer,
                           void** producer_data,
                           void* handler_data)
    {
        *producer = server_producer;
        *producer_data = handler_data;
        return 0;
    }

    static
    int
    server_handler_finished( void* data, int success )
    {
        return 0;
    }

    
    static
    int
    server_periodic( void* arg )
    {
        SendRecvFixtureBase* that = (SendRecvFixtureBase*)arg;
        assert( pthread_mutex_lock( &that->lock ) == 0 );
        if( that->m_server_runs_flag == 0 ) {
            that->m_server_runs_flag = 1;
            assert( pthread_cond_signal( &that->m_server_runs_cond ) == 0 );
        }
        assert( pthread_mutex_unlock( &that->lock ) == 0 );
        return 0;
    }
    
    static
    void*
    server_thread_func( void* arg )
    {
        SendRecvFixtureBase* that = (SendRecvFixtureBase*)arg;
        // setup server
        tinia_ipc_msg_server_t* server = ipc_msg_server_create( "unittest",
                                                  logger,
                                                  arg );
        {
            assert( pthread_mutex_lock( &that->lock ) == 0 );
            BOOST_REQUIRE( server != NULL );
            BOOST_REQUIRE( server->shmem_base != NULL );
            BOOST_REQUIRE( server->shmem_header_ptr != NULL );
            that->m_server = server;
            assert( pthread_mutex_unlock( &that->lock ) == 0  );
        }
        // run server mainloop
        //std::cerr << __LINE__ << ": A\n";
        int rc = ipc_msg_server_mainloop( server,
                                          server_periodic, that,
                                          server_input_handler, that,
                                          server_output_handler, that );
        //std::cerr << __LINE__ << ": B\n";
        // teardown server
        {
            assert( pthread_mutex_lock( &that->lock ) == 0 );
            BOOST_CHECK( rc == 0 );
            that->m_server = NULL;
            assert( pthread_mutex_unlock( &that->lock ) == 0  );
        }
        rc = ipc_msg_server_delete( server );
        {
            assert( pthread_mutex_lock( &that->lock ) == 0 );
            BOOST_CHECK( rc == 0 );
            assert( pthread_cond_signal( &that->m_server_runs_cond ) == 0 );
            assert( pthread_mutex_unlock( &that->lock ) == 0  );
        }
        return NULL;
    }
    
 
    
    static
    int
    client_producer( void* data,
                     int* more,
                     char* buffer,
                     size_t* buffer_bytes,
                     const size_t buffer_size,
                     const int part )
    {
        return ((SendRecvFixtureBase*)data)->clientProducer( more,
                                                             buffer,
                                                             buffer_bytes,
                                                             buffer_size,
                                                             part );
    }

    
    
    static
    int
    client_consumer( void* data,
                     const char* buffer,
                     const size_t buffer_bytes,
                     const int part,
                     const int more )
    {
        return ((SendRecvFixtureBase*)data)->clientConsumer( buffer,
                                                             buffer_bytes,
                                                             part,
                                                             more );
    }

    
    static
    void*
    client_thread_func( void* arg )
    {
        int rc;
        SendRecvFixtureBase* that = (SendRecvFixtureBase*)arg;
        
        assert( pthread_mutex_lock( &that->lock ) == 0 );
        that->m_clients_initialized++;
        if( that->m_clients_initialized == that->m_clients ) {
            // last client to finish init
            BOOST_REQUIRE( pthread_cond_signal( &that->m_clients_initialized_cond ) == 0 );
        }
        assert( pthread_mutex_unlock( &that->lock ) == 0  );

        tinia_ipc_msg_client_t* client = (tinia_ipc_msg_client_t*)malloc( tinia_ipc_msg_client_t_sizeof );
        rc = ipc_msg_client_init( client, "unittest", logger, arg );
        {
            assert( pthread_mutex_lock( &that->lock ) == 0 );
            BOOST_REQUIRE( rc == 0 );
            assert( pthread_mutex_unlock( &that->lock ) == 0  );
        }
        do {
            assert( pthread_mutex_lock( &that->lock ) == 0 );
            int timeout = that->m_clients_should_longpoll ? 2 : 0;
            assert( pthread_mutex_unlock( &that->lock ) == 0  );

            rc = ipc_msg_client_sendrecv( client,
                                          client_producer, that,
                                          client_consumer, that,
                                          timeout );
            
            assert( pthread_mutex_lock( &that->lock ) == 0 );
            if( that->m_failure_is_an_option ) {
                BOOST_REQUIRE_GE( rc, -1 );
            }
            else {
                BOOST_REQUIRE_EQUAL( rc, 0 );
            }
            assert( pthread_mutex_unlock( &that->lock ) == 0  );
        } while(0);
        rc = ipc_msg_client_release( client );
        assert( pthread_mutex_lock( &that->lock ) == 0 );
        BOOST_REQUIRE( rc == 0 );
        assert( pthread_mutex_unlock( &that->lock ) == 0  );
        
        free( client );
        
        // notify that we have finished
        assert( pthread_mutex_lock( &that->lock ) == 0 );
        that->m_clients_exited++;
        if( that->m_clients_exited == that->m_clients ) {
            // last client to finish init
            BOOST_REQUIRE( pthread_cond_signal( &that->m_clients_exited_cond ) == 0 );
        }
        assert( pthread_mutex_unlock( &that->lock ) == 0  );
        return NULL;
    }


    
};

