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
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <boost/test/unit_test.hpp>
#include <tinia/ipc/ipc_msg.h>
#include "../../src/ipc/ipc_msg_internal.h"


#define NOT_MAIN_THREAD_REQUIRE( obj, a ) do { if(!(a)){                    \
    std::stringstream o;                                                    \
    o << (#a) << " failed at " << __FILE__ << '@' << __LINE__;              \
    obj->setErrorFromThread( o.str() );                                     \
    } } while(0)

#define FAIL_MISERABLY_UNLESS( a ) do { if(!(a) ) {                             \
    fprintf( stderr, "Failed misrably at %s@%d: %s\n", __FILE__, __LINE__, #a );\
    *((int*)0xDEADBEEF) = 42;                                                   \
    } } while(0)

struct SendRecvFixtureBase;




// RAII-locker for fixture
class Locker
{
public:
    explicit Locker( pthread_mutex_t& mutex )
        : m_mutex( mutex ),
          m_locked( false )
    {
        lock();
    }

    ~Locker()
    {
        if( m_locked ) {
            unlock();
        }
    }

    void
    lock()
    {
        if( m_locked ) {
            fprintf( stderr, "Locker::lock: Locker already locked\n" );
            *((int*)0xDEADBEEF) = 42;   // Serious error, force segfault & termination.
        }

        int rv = pthread_mutex_lock( &m_mutex );
        if( rv != 0 ) {
            fprintf( stderr, "Locker::lock: %s\n", strerror( rv ) );
        }
        m_locked = true;
    }

    void
    unlock()
    {
        if( !m_locked ) {
            fprintf( stderr, "Locker::lock: Locker not locked\n" );
            *((int*)0xDEADBEEF) = 42;   // Serious error, force segfault & termination.
        }

        int rv = pthread_mutex_unlock( &m_mutex );
        if( rv != 0 ) {
            fprintf( stderr, "Locker::lock: %s\n", strerror( rv ) );
        }
        m_locked = false;
    }

protected:
    pthread_mutex_t&        m_mutex;
    bool                    m_locked;
};

class ScopeTrace
{
public:
    explicit inline ScopeTrace( SendRecvFixtureBase* that, const std::string& what );

    inline ~ScopeTrace();

protected:
    SendRecvFixtureBase*    m_that;
    const std::string       m_what;
    pthread_t               m_id;
    int                     m_index;
};




struct SendRecvFixtureBase
{



    SendRecvFixtureBase()
        : m_server( NULL ),
          m_server_runs_flag( 0 ),
          m_failure_is_an_option( 0 ),
          m_clients( 1 ),
          m_clients_should_longpoll( 0 ),
          m_clients_initialized( 0 ),
          m_clients_exited( 0 ),
          m_jitter(1000)
    {}
    
    std::vector<pthread_t>  m_threads;
    pthread_mutex_t         m_threads_lock;

    pthread_mutex_t         lock;
    pthread_mutex_t         client_lock;    // for use when transaction lock is held
    pthread_mutex_t         server_lock;    // for use when transaction lock is held
    
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
    std::string             m_error_from_thread;
    int                     m_jitter;

    
    void
    setErrorFromThread( const std::string& error )
    {
        Locker locker( this->lock );
        m_error_from_thread = error;
    }

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
#ifdef TINIA_IPC_LOG_TRACE
        fprintf( stderr, "--- [FIXTURE] test begin ------------------------------------------------------\n" );
#endif
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
        BOOST_REQUIRE( pthread_mutexattr_settype( &mutex_attr, PTHREAD_MUTEX_RECURSIVE ) == 0 );
        
        BOOST_REQUIRE( pthread_mutex_init( &lock, &mutex_attr ) == 0 );

        pthread_mutexattr_t mutex_attr2;
        BOOST_REQUIRE( pthread_mutexattr_init( &mutex_attr2) == 0 );
        BOOST_REQUIRE( pthread_mutexattr_settype( &mutex_attr2, PTHREAD_MUTEX_ERRORCHECK ) == 0 );
        BOOST_REQUIRE( pthread_mutex_init( &client_lock, &mutex_attr2 ) == 0 );
        BOOST_REQUIRE( pthread_mutex_init( &server_lock, &mutex_attr2 ) == 0 );
        BOOST_REQUIRE( pthread_mutex_init( &m_threads_lock, &mutex_attr2 ) == 0 );

        BOOST_REQUIRE( pthread_cond_init( &m_server_runs_cond, NULL ) == 0 );
        BOOST_REQUIRE( pthread_cond_init( &m_clients_initialized_cond, NULL ) == 0 );
        BOOST_REQUIRE( pthread_cond_init( &m_clients_exited_cond, NULL ) == 0 );
        
        BOOST_REQUIRE( pthread_mutexattr_destroy( &mutex_attr) == 0 );

        // wait for server to be up and running.
        Locker locker( this->lock );
        m_server = NULL;

        {   Locker threads_lock( m_threads_lock );
            m_threads.resize( m_threads.size()+1 );
            BOOST_REQUIRE( pthread_create( &m_threads.back(),
                                           NULL,
                                           server_thread_func,
                                           this ) == 0 );
        }
        m_server_runs_flag = 0;
        {
            ScopeTrace scope_trace( this, std::string(__func__)+".scope_0" );
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
        }

        BOOST_REQUIRE( m_server != NULL );
        BOOST_REQUIRE( m_server->shmem_header_ptr != NULL );
            
        // create clients
        BOOST_REQUIRE( (m_clients >= 0) && (m_clients < 10) ); // sanity check
        for(int i=0; i<m_clients; i++ ) {
            Locker threads_lock( m_threads_lock );
            m_threads.resize( m_threads.size()+1 );
            BOOST_REQUIRE( pthread_create( &m_threads.back(),
                                           NULL,
                                           client_thread_func,
                                           this ) == 0 );
        }

        
        // wait for clients to run
        {
            ScopeTrace scope_trace( this, std::string(__func__)+".scope_1" );
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
        }

        if( inner() != 0 ) {
            goto hung;
        }

        // wait for clients to finish
        {
            ScopeTrace scope_trace( this, std::string(__func__)+".scope_2" );
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
        }

        // kill server
        BOOST_REQUIRE( m_server != NULL );

        locker.unlock();
        rc = ipc_msg_server_mainloop_break( m_server );
        BOOST_REQUIRE( rc == 0 );

        locker.lock();
        
        // wait for server thread to finsh
        {
            ScopeTrace scope_trace( this, std::string(__func__)+".scope_3" );
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
        }
        locker.unlock();
        goto cleanup;
hung:
        ipc_msg_server_mainloop_break( m_server );
        usleep( 100000 );

        // invariants:
        // - m_shared_lock locked.
        {   Locker threads_lock( m_threads_lock );
            for( size_t i=0; i<m_threads.size(); i++ ) {
                pthread_cancel( m_threads[i] );
            }
        }
        locker.unlock();

cleanup:

        for( int it=0; it<3; it++) {
            Locker threads_lock( m_threads_lock );
            std::vector<pthread_t> unterminated;
            for( size_t i=0; i<m_threads.size(); i++ ) {
                void* tmp;
                if( pthread_tryjoin_np( m_threads[i], &tmp ) != 0 ) {
                    unterminated.push_back( m_threads[i] );
                }
            }
            m_threads.swap( unterminated );
            if( m_threads.empty() ) {
                if( it != 0 ) {
                    fprintf( stderr, "FIXTURE: All threads joined.\n" );
                }
                goto done;
            }

            int msec = 1<<(10*it);
            fprintf( stderr, "FIXTURE: %d threads still alive, waiting %d microseconds.\n",
                     (int)m_threads.size(), msec  );
            usleep( msec );
            if( it != 0 ) {
                fprintf( stderr, "FIXTURE: Cancelling threads and retrying to join them.\n" );
                for( size_t i=0; i<m_threads.size(); i++ ) {
                    pthread_cancel( m_threads[i] );
                }
            }
        }
        FAIL_MISERABLY_UNLESS( 0 && "Unable to join threads." );
done:
        {
            Locker threads_lock( m_threads_lock );
            BOOST_REQUIRE( m_threads.empty() );
        }
        //std::cerr << "done.\n";
        BOOST_REQUIRE( pthread_cond_destroy( &m_server_runs_cond ) == 0 );
        BOOST_REQUIRE( pthread_cond_destroy( &m_clients_initialized_cond ) == 0 );
        BOOST_REQUIRE( pthread_cond_destroy( &m_clients_exited_cond) == 0 );
        FAIL_MISERABLY_UNLESS( pthread_mutex_destroy( &lock ) == 0 );
        //ipc_msg_server_wipe( "unittest" );
        if( !m_error_from_thread.empty() ) {
            fprintf( stderr, "FIXTURE: Error from thread: %s\n", m_error_from_thread.c_str() );
            BOOST_REQUIRE( m_error_from_thread.empty() );
        }

        fprintf( stderr, "--- [FIXTURE] test end --------------------------------------------------------\n" );
    }

    static
    void
    logger( void* data,
            int level,
            const char* who,
            const char* msg, ... )
    {
        SendRecvFixtureBase* that = (SendRecvFixtureBase*)data;
        Locker locker( that->lock );
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
    server_input_handler( tinia_ipc_msg_consumer_func_t* consumer_, void** consumer_data,
                          void* handler_data,
                          const char* buffer,
                          const size_t buffer_bytes )
    {
        *consumer_ = server_consumer;
        *consumer_data = handler_data;
        return 0;
    }

    static
    int
    server_output_handler( tinia_ipc_msg_producer_func_t* producer,
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
        unsigned int seed = 0;
        if( that->m_jitter ) {
            seed = get_random_seed();
            int time = ( (long)that->m_jitter*rand_r( &seed )) /RAND_MAX;
            usleep( time );
        }


        Locker locker( that->lock );
        if( that->m_server_runs_flag == 0 ) {
            that->m_server_runs_flag = 1;
            NOT_MAIN_THREAD_REQUIRE( that, pthread_cond_signal( &that->m_server_runs_cond ) == 0 );
        }
        return 0;
    }
    
    static
    void*
    server_thread_func( void* arg )
    {
        SendRecvFixtureBase* that = (SendRecvFixtureBase*)arg;
        ScopeTrace scope_trace( that, __func__ );

        // setup server
        tinia_ipc_msg_server_t* server = ipc_msg_server_create( "unittest",
                                                  logger,
                                                  arg );
        {
            ScopeTrace scope_trace( that, std::string(__func__)+".scope_0" );
            Locker locker( that->lock );
            NOT_MAIN_THREAD_REQUIRE( that, server != NULL );
            NOT_MAIN_THREAD_REQUIRE( that, server->shmem_base != NULL );
            NOT_MAIN_THREAD_REQUIRE( that, server->shmem_header_ptr != NULL );
            that->m_server = server;
        }
        // run server mainloop
        //std::cerr << __LINE__ << ": A\n";

        int rc;
        {
            ScopeTrace scope_trace( that, std::string(__func__)+".scope_1" );
            rc = ipc_msg_server_mainloop( server,
                                          server_periodic, that,
                                          server_input_handler, that,
                                          server_output_handler, that );
        }
        if( rc < -1 ) { // Serious error.
            fprintf( stderr, "Return code from ipc_msg_server_main = %d\n", rc );
            NOT_MAIN_THREAD_REQUIRE( that, rc >= -1 );
        }
        {
            ScopeTrace scope_trace( that, std::string(__func__)+".scope_2" );
            Locker locker( that->lock );
            that->m_server = NULL;
        }

        {
            ScopeTrace scope_trace( that, std::string(__func__)+".scope_3" );
            rc = ipc_msg_server_delete( server );
        }
        NOT_MAIN_THREAD_REQUIRE( that, rc == 0 );
        {
            ScopeTrace scope_trace( that, std::string(__func__)+".scope_4" );
            Locker locker( that->lock );
            NOT_MAIN_THREAD_REQUIRE( that, pthread_cond_signal( &that->m_server_runs_cond ) == 0 );
        }
        return NULL;
    }
    
    static
    unsigned int
    get_random_seed()
    {
/*
        unsigned int ret;
        FILE* urandom = fopen( "/dev/urandom", "r" );
        if( fread( &ret, sizeof(ret), 1, urandom ) != sizeof(ret) ) {
            fprintf( stderr, "Failed reading /dev/urandom.\n" );
        }
        fclose( urandom );
        return ret;
*/
        struct timeval t;
        gettimeofday( &t, NULL );
        return (unsigned int)t.tv_usec + (unsigned int)pthread_self();
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
    int
    client_input_handler( tinia_ipc_msg_consumer_func_t* consumer_, void** consumer_data,
                          void* handler_data,
                          const char* buffer,
                          const size_t buffer_bytes )
    {
        *consumer_ = client_consumer;
        *consumer_data = handler_data;
        return 0;
    }

    
    static
    void*
    client_thread_func( void* arg )
    {
        int rc;
        SendRecvFixtureBase* that = (SendRecvFixtureBase*)arg;
        ScopeTrace scope_trace_body( that, __func__ );
        
        unsigned int seed = 0;
        if( that->m_jitter ) {
            seed = get_random_seed();
            int time = ( (long)that->m_jitter*rand_r( &seed )) /RAND_MAX;
            usleep( time );
        }

        {
            ScopeTrace scope_trace( that, std::string(__func__)+".scope_0" );
            Locker locker( that->lock );
            that->m_clients_initialized++;
            if( that->m_clients_initialized == that->m_clients ) {
                // last client to finish init
                NOT_MAIN_THREAD_REQUIRE( that, pthread_cond_signal( &that->m_clients_initialized_cond ) == 0 );
            }
        }

        tinia_ipc_msg_client_t* client = (tinia_ipc_msg_client_t*)malloc( tinia_ipc_msg_client_t_sizeof );
        rc = tinia_ipc_msg_client_init( client, "unittest", logger, arg );
        NOT_MAIN_THREAD_REQUIRE( that, rc == 0 );

        do {
            ScopeTrace scope_trace( that, std::string(__func__)+".scope_1" );
            if( that->m_jitter ) {
                seed = get_random_seed();
                int time = ( (long)that->m_jitter*rand_r( &seed )) /RAND_MAX;
                usleep( time );
            }

            int timeout;
            bool failure_is_an_option;
            {
                Locker locker( that->lock );
                timeout = that->m_clients_should_longpoll ? 2 : 0;
                failure_is_an_option = that->m_failure_is_an_option;
            }

            rc = tinia_ipc_msg_client_sendrecv( client,
                                          client_producer, that,
                                          client_consumer, that,
                                          timeout );
            
            if( failure_is_an_option ) {
                NOT_MAIN_THREAD_REQUIRE( that, rc >= -1 );
            }
            else {
                NOT_MAIN_THREAD_REQUIRE( that, rc == 0 );
            }

        } while(0);

        rc = tinia_ipc_msg_client_release( client );
        NOT_MAIN_THREAD_REQUIRE( that, rc == 0 );
        free( client );
        
        // notify that we have finished
        {
            ScopeTrace scope_trace( that, std::string(__func__)+".scope_2" );
            Locker locker( that->lock );
            that->m_clients_exited++;
            if( that->m_clients_exited == that->m_clients ) {
                // last client to finish init
                NOT_MAIN_THREAD_REQUIRE( that, pthread_cond_signal( &that->m_clients_exited_cond ) == 0 );
            }
        }

        return NULL;
    }


    
};


ScopeTrace::ScopeTrace( SendRecvFixtureBase* that, const std::string& what )
    : m_that( that ),
      m_what( what ),
      m_id( pthread_self() ),
      m_index( -1 )

{
#ifdef TINIA_IPC_LOG_TRACE
    {
        pthread_t id = pthread_self();
        Locker threads_locker( that->m_threads_lock );
        for( size_t i=0; i<that->m_threads.size(); i++ ) {
            if( that->m_threads[i] == id ) {
                m_index = i;
                break;
            }
        }
    }

    Locker locker( m_that->lock );
    fprintf( stderr, "    [%lu | %d] >>> %s.\n", m_id, m_index, m_what.c_str() );
#endif
}

ScopeTrace::~ScopeTrace()
{
#ifdef TINIA_IPC_LOG_TRACE
    Locker locker( m_that->lock );
    fprintf( stderr, "    [%lu | %d] <<< %s.\n", m_id, m_index, m_what.c_str() );
#endif
}


