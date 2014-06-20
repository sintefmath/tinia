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
#ifdef TINIA_IPC_VALGRIND_ANNOTATIONS
#include <helgrind.h>
#include <drd.h>
#else
// No valigrind annotations; empty macros
#define VALGRIND_HG_BARRIER_INIT_PRE(_bar, _count, _resizable)
#define VALGRIND_HG_BARRIER_WAIT_PRE(_bar)
#define VALGRIND_HG_BARRIER_RESIZE_PRE(_bar, _newcount)
#define VALGRIND_HG_BARRIER_DESTROY_PRE(_bar)
#define ANNOTATE_HAPPENS_BEFORE( obj )
#define ANNOTATE_HAPPENS_AFTER( obj )
#endif

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
          m_jitter(1000)
    {}
    
    std::vector<pthread_t>  m_threads;
    pthread_mutex_t         m_threads_lock;

    pthread_barrier_t       m_barrier_server_running;
    pthread_barrier_t       m_barrier_server_finished;
    pthread_barrier_t       m_barrier_clients_running;
    pthread_barrier_t       m_barrier_clients_finished;
    pthread_barrier_t       m_barrier_finished_with_msgserver;

    // In the end of the test, we (at least try) to join all threads that we
    // create during the test (server and clients). When successful, this in
    // effect implements a barrier. However, helgrind doesn't seem to pick up
    // on this, and generates some false positives. To avoid this, we annotate
    // this implicit barrier using this variable.
    int                     m_implicit_join_threads_barrier;
    
    pthread_mutex_t         lock;
    pthread_mutex_t         client_lock;    // for use when transaction lock is held
    pthread_mutex_t         server_lock;    // for use when transaction lock is held
    
    tinia_ipc_msg_server_t*               m_server;
    int                     m_server_runs_flag;
    int                     m_failure_is_an_option;
    int                     m_clients;
    int                     m_clients_should_longpoll;

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

        
        BOOST_REQUIRE( pthread_barrier_init( &m_barrier_server_running, NULL, 2 ) == 0 );
        BOOST_REQUIRE( pthread_barrier_init( &m_barrier_server_finished, NULL, 2 ) == 0 );
        BOOST_REQUIRE( pthread_barrier_init( &m_barrier_clients_running, NULL, m_clients + 1 ) == 0 );
        BOOST_REQUIRE( pthread_barrier_init( &m_barrier_clients_finished, NULL, m_clients + 1 ) == 0 );
        BOOST_REQUIRE( pthread_barrier_init( &m_barrier_finished_with_msgserver, NULL, 2 ) == 0 );


        VALGRIND_HG_BARRIER_INIT_PRE( &m_implicit_join_threads_barrier, (m_clients+2), 0 );

        BOOST_REQUIRE( pthread_mutexattr_destroy( &mutex_attr) == 0 );

        // --- run server and wait for it be up and running --------------------
        Locker locker( this->lock );
        m_server = NULL;
        m_server_runs_flag = 0;
        {   Locker threads_lock( m_threads_lock );
            m_threads.resize( m_threads.size()+1 );
            BOOST_REQUIRE( pthread_create( &m_threads.back(),
                                           NULL,
                                           server_thread_func,
                                           this ) == 0 );
        }
        locker.unlock();
        {
            ScopeTrace scope_trace( this, std::string(__func__) + ".barrier_server_running" );
            rc = pthread_barrier_wait( &m_barrier_server_running );
            BOOST_REQUIRE( (rc==0) || (rc==PTHREAD_BARRIER_SERIAL_THREAD ) );
        }
        locker.lock();

        BOOST_REQUIRE( m_server != NULL );
        BOOST_REQUIRE( m_server->shmem_header_ptr != NULL );
            
        // --- create clients and wait for all of them to be up and running ----
        BOOST_REQUIRE( (m_clients >= 0) && (m_clients < 10) ); // sanity check
        for(int i=0; i<m_clients; i++ ) {
            Locker threads_lock( m_threads_lock );
            m_threads.resize( m_threads.size()+1 );
            BOOST_REQUIRE( pthread_create( &m_threads.back(),
                                           NULL,
                                           client_thread_func,
                                           this ) == 0 );
        }
        locker.unlock();

        {
            ScopeTrace scope_trace( this, std::string(__func__) + ".barrier_clients_running" );
            rc = pthread_barrier_wait( &m_barrier_clients_running );
            BOOST_REQUIRE( (rc==0) || (rc==PTHREAD_BARRIER_SERIAL_THREAD ) );
        }

        locker.lock();
        int inner_rc = inner();
        locker.unlock();

        if( inner_rc != 0 ) {
            fprintf( stderr, "FIXTURE: Entering error handling path...\n" );
            // something went wrong, just cancel all threads and hope for the best
            ipc_msg_server_mainloop_break( m_server );
            usleep( 100000 );
    
            // invariants:
            // - m_shared_lock locked.
            {   Locker threads_lock( m_threads_lock );
                for( size_t i=0; i<m_threads.size(); i++ ) {
                    pthread_cancel( m_threads[i] );
                }
            }
        }
        else {
            // wait for clients to finish
            ScopeTrace scope_trace( this, std::string(__func__) + ".barrier_clients_finished" );
            rc = pthread_barrier_wait( &m_barrier_clients_finished );
            BOOST_REQUIRE( (rc==0) || (rc==PTHREAD_BARRIER_SERIAL_THREAD ) );

            // --- ask server to quit and wait until server thread finishes ----
            rc = ipc_msg_server_mainloop_break( m_server );
            BOOST_REQUIRE( rc == 0 );
            {
                ScopeTrace scope_trace( this, std::string(__func__)+".barrier_finished_with_msgserver"  );
                int rc = pthread_barrier_wait( &m_barrier_finished_with_msgserver );
                BOOST_REQUIRE( (rc == 0) || (rc == PTHREAD_BARRIER_SERIAL_THREAD) );
            }

            {
                ScopeTrace scope_trace( this, std::string(__func__)+".barrier_server_finished" );
    
                int rc = pthread_barrier_wait( &m_barrier_server_finished );
                BOOST_REQUIRE( (rc == 0) || (rc == PTHREAD_BARRIER_SERIAL_THREAD) );
            }
        }
        
        // ---- and wait around until all threads actually terminates ----------
        VALGRIND_HG_BARRIER_WAIT_PRE( &m_implicit_join_threads_barrier );       // for helgrind
        ANNOTATE_HAPPENS_AFTER( &m_implicit_join_threads_barrier );             // for drd

        std::vector<pthread_t> threads;
        {
            Locker threads_lock( m_threads_lock );
            threads = m_threads;
        }

        for( size_t i=0; i<threads.size(); i++ ) {
            struct timespec timeout;
            BOOST_REQUIRE( clock_gettime( CLOCK_REALTIME, &timeout ) == 0 );
            timeout.tv_sec += 60;

            void* ret;
            int rc = pthread_timedjoin_np( threads[i], &ret, &timeout );
            if( rc != 0 ) {
                fprintf( stderr, "pthread_join: %s", strerror(rc) );
                BOOST_REQUIRE( 0 );
            }
        }

        VALGRIND_HG_BARRIER_DESTROY_PRE( &m_implicit_join_threads_barrier );

        BOOST_REQUIRE( pthread_barrier_destroy( &m_barrier_server_running ) == 0 );
        BOOST_REQUIRE( pthread_barrier_destroy( &m_barrier_server_finished ) == 0 );
        BOOST_REQUIRE( pthread_barrier_destroy( &m_barrier_clients_running ) == 0 );
        BOOST_REQUIRE( pthread_barrier_destroy( &m_barrier_clients_finished ) == 0 );
        BOOST_REQUIRE( pthread_barrier_destroy( &m_barrier_finished_with_msgserver ) == 0 );
        
        BOOST_REQUIRE( pthread_mutex_destroy( &lock ) == 0 );
        BOOST_REQUIRE( pthread_mutex_destroy( &client_lock ) == 0 );
        BOOST_REQUIRE( pthread_mutex_destroy( &server_lock ) == 0 );
        BOOST_REQUIRE( pthread_mutex_destroy( &m_threads_lock ) == 0 );
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
        ScopeTrace scope_trace( that, __func__ );
        unsigned int seed = 0;
        if( that->m_jitter ) {
            seed = get_random_seed();
            int time = ( (long)that->m_jitter*rand_r( &seed )) /RAND_MAX;
            usleep( time );
        }


        // First invocation of server, notify that server is running.
        int flag = 0;
        {
            Locker locker( that->server_lock );
            flag = that->m_server_runs_flag;
            if( flag == 0 ) {
                that->m_server_runs_flag = 1;
            }
        }
        if( flag == 0 ) {
            ScopeTrace scope_trace( that, std::string(__func__) + ".barrier_server_running" );
            int rc = pthread_barrier_wait( &that->m_barrier_server_running );
            NOT_MAIN_THREAD_REQUIRE( that, (rc == 0) || (rc == PTHREAD_BARRIER_SERIAL_THREAD) );
        }
        return 0;
    }
    
    static
    void*
    server_thread_func( void* arg )
    {
        SendRecvFixtureBase* that = (SendRecvFixtureBase*)arg;
        {   // Extra scope to assert that VALGRIND_HG_BARRIER_WAIT_PRE runs last
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
            // wait until all is finished with server
            {
                ScopeTrace scope_trace( that, std::string(__func__)+".barrier_finished_with_msgserver" );
                int rc=pthread_barrier_wait( &that->m_barrier_finished_with_msgserver );
                NOT_MAIN_THREAD_REQUIRE( that, (rc == 0) || (rc == PTHREAD_BARRIER_SERIAL_THREAD) );
            }
            {
                ScopeTrace scope_trace( that, std::string(__func__)+".scope_2" );
                Locker locker( that->lock );
                that->m_server = NULL;
            }

            {
                ScopeTrace scope_trace( that, std::string(__func__)+".scope_3" );
                rc = ipc_msg_server_delete( server );
                NOT_MAIN_THREAD_REQUIRE( that, rc == 0 );
            }
            {
                ScopeTrace scope_trace( that, std::string(__func__)+".barrier_server_finished" );
                int rc=pthread_barrier_wait( &that->m_barrier_server_finished );
                NOT_MAIN_THREAD_REQUIRE( that, (rc == 0) || (rc == PTHREAD_BARRIER_SERIAL_THREAD) );
            }
        }
        VALGRIND_HG_BARRIER_WAIT_PRE( &that->m_implicit_join_threads_barrier ); // for helgrind
        ANNOTATE_HAPPENS_BEFORE( &that->m_implicit_join_threads_barrier );      // for drd
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
        SendRecvFixtureBase* that = (SendRecvFixtureBase*)arg;
        {   // Extra scope to assert that VALGRIND_HG_BARRIER_WAIT_PRE runs last
            int rc;
            ScopeTrace scope_trace_body( that, __func__ );

            unsigned int seed = 0;
            if( that->m_jitter ) {
                seed = get_random_seed();
                int time = ( (long)that->m_jitter*rand_r( &seed )) /RAND_MAX;
                usleep( time );
            }

            {
                ScopeTrace scope_trace( that, std::string(__func__)+".barrier_clients_running" );
                int rc = pthread_barrier_wait( &that->m_barrier_clients_running );
                NOT_MAIN_THREAD_REQUIRE( that, (rc==0) || (rc==PTHREAD_BARRIER_SERIAL_THREAD) );
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

            {
                ScopeTrace scope_trace( that, std::string(__func__)+".barrier_clients_finished" );
                int rc = pthread_barrier_wait( &that->m_barrier_clients_finished );
                NOT_MAIN_THREAD_REQUIRE( that, (rc==0) || (rc==PTHREAD_BARRIER_SERIAL_THREAD) );
            }
        }
        VALGRIND_HG_BARRIER_WAIT_PRE( &that->m_implicit_join_threads_barrier ); // for helgrind
        ANNOTATE_HAPPENS_BEFORE( &that->m_implicit_join_threads_barrier );      // for drd
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


