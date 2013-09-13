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

#include <sys/stat.h>   // mode constants
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <list>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>      // O_* constants
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include "tinia/trell/IPCController.hpp"
#include "tinia/ipc/messenger.h"

namespace tinia {
namespace trell {

namespace {
static const std::string package = "IPCController";

static sig_atomic_t exit_flag = 0;

static
void
handle_SIGTERM( int )
{
    exit_flag = 1;
    std::cerr << "handle_SIGTERM\n";
}

static
void
handle_SIGALRM( int )
{
    std::cerr << "handle_SIGALRM\n";
}


}



IPCController::IPCController( bool is_master )
    : m_ipc_pid( getpid() ),
      m_cleanup_pid( -1 ),
      m_is_master( is_master ),
      m_job_state( TRELL_JOBSTATE_NOT_STARTED ),
      m_shmem_ptr( MAP_FAILED ),
      m_shmem_size( 100*1024*1024 ),
      m_sem_lock( SEM_FAILED ),
      m_sem_query( SEM_FAILED ),
      m_sem_reply( SEM_FAILED ),
      m_notify( false ),
      m_sem_notify( SEM_FAILED )
{
    //instances.push_back(this);
}

void
IPCController::finish()
{
    m_job_state = TRELL_JOBSTATE_FINISHED;
    kill( getpid(), SIGCONT );
}

void
IPCController::fail()
{
    m_job_state = TRELL_JOBSTATE_FAILED;
    kill( getpid(), SIGCONT );
}

void
IPCController::failHard()
{
    fail();
    shutdown();
}

void
IPCController::notify()
{
    // This function is called by the job thread. My initial plan was to set a
    // notify flag and raise a signal, which in theory, would interrupt the
    // sem_timedwait run by the IPC thread. The IPC thread would then be able to
    // notify. However, it doesn't seem like sem_timewait is interrupted.
    //

    int value;
    if( sem_trywait( m_sem_lock ) == 0 ) {
        if( sem_getvalue( m_sem_notify, &value ) == 0 ) {
            for( int i=value; i<1; i++) {
                sem_post( m_sem_notify );
            }
        }
        sem_post( m_sem_lock );
    }
    else {
        // we didn't get the lock, we let the IPC thread do the notification
        // when it is finished with the request
        m_notify = true;
    }
}

void IPCController::addScript(const std::string &script)
{
    m_scripts.push_back(script);
}

bool IPCController::onGetScripts(size_t &result_size, char *buffer, const size_t buffer_size)
{
    // Initialize result
    result_size = 0;
    std::string header("");
    header.copy(buffer, buffer_size);
    buffer += header.size();
    result_size += header.size();
    for(size_t i = 0; i < m_scripts.size(); ++i) {
        m_scripts[i].copy(buffer, buffer_size);
        buffer += m_scripts[i].size();
        result_size += m_scripts[i].size();
        if(result_size > buffer_size) {
            return false;
        }
    }
    return true;
}


bool
IPCController::createSharedMemory( void** memory,
                                size_t* memory_size,
                                const std::string& name,
                                const size_t size )
{
    *memory = MAP_FAILED;
    *memory_size = 0u;

    // first, kill memory with this name if exists,
    shm_unlink( name.c_str() );

    // create and open
    int fd = shm_open( name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0666 );
    if( fd < 0 ) {
        std::cerr << "Failed to create shared memory: " << strerror( errno ) << std::endl;
        deleteSharedMemory( memory, memory_size, name );
        return false;
    }

    // set size
    if( ftruncate( fd, size ) != 0 ) {
        std::cerr << "Failed to set shared memory size: " << strerror( errno ) << std::endl;
        close( fd );
        deleteSharedMemory( memory, memory_size, name );
        return false;
    }

    // query actual size
    struct stat fstat_buf;
    if( fstat( fd, &fstat_buf ) != 0 ) {
        std::cerr << "Failed to determine actual shared memory size: " << strerror(errno) << std::endl;
        close( fd );
        deleteSharedMemory( memory, memory_size, name );
        return false;
    }
    *memory_size = fstat_buf.st_size;

    // map memory
    *memory = mmap( NULL, *memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    if( *memory == MAP_FAILED ) {
        std::cerr << "Failed to map shared memory: " << strerror( errno ) << std::endl;
        close( fd );
        deleteSharedMemory( memory, memory_size, name );
        return false;
    }
    close( fd );
    return true;
}


void
IPCController::deleteSharedMemory( void** memory, size_t* memory_size, const std::string& name )
{
    if( *memory != MAP_FAILED ) {
        if( munmap( *memory, *memory_size ) != 0 ) {
            std::cerr << "Failed to unmap shared memory: " << strerror( errno ) << std::endl;
        }
    }
    if( !name.empty() ) {
        shm_unlink( name.c_str() );
    }
    *memory = MAP_FAILED;
    *memory_size = 0u;
}


bool
IPCController::createSemaphore( sem_t** semaphore, const std::string& name )
{
    // wipe old semaphore with same name, if exists.
    sem_unlink( name.c_str() );

    // open semaphore
    *semaphore = sem_open( name.c_str(), O_CREAT | O_EXCL, 0666, 0 );
    if( *semaphore == SEM_FAILED ) {
        std::cerr << "Failed to create semaphore: " << strerror( errno ) << std::endl;
        deleteSemaphore( semaphore, name );
        return false;
    }
    return true;
}

void
IPCController::deleteSemaphore( sem_t** semaphore, const std::string& name )
{
    if( *semaphore != SEM_FAILED ) {
        sem_close( *semaphore );
    }
    if( !name.empty() ) {
        sem_unlink( name.c_str() );
    }
    *semaphore = SEM_FAILED;
}

void
IPCController::shutdown()
{
    if( m_cleanup_pid != getpid() ) {
        // The Master (which subclass this class) forks, and the child inherits
        // signal handlers and open io stuff. If the subsequent exec fails, the
        // child terminates and the signal handler is invoked, which in turn
        // closes the parent's message box. To avoid this, we only allow the
        // process that created the message box to clean it up. Also, by setting
        // the cleanup-pid to -1 after we've finished cleaning up avoids trying
        // to cleanup multiple times (we have both a signal handler and an
        // at_exit-handler).
        return;
    }
    if( m_job_state != TRELL_JOBSTATE_TERMINATED_SUCCESSFULLY ) {
        m_job_state = TRELL_JOBSTATE_TERMINATED_UNSUCCESSFULLY;
    }
    sendHeartBeat();
    std::cerr << "Cleaning up.\n";
    cleanup();

    if( !m_is_master ) {
        messenger_status_t mrv = messenger_free( &m_master_mbox );
        if( mrv != MESSENGER_OK ) {
            std::cerr << "Failed to close messenger: " << messenger_strerror( mrv ) << "\n";
        }
    }

    deleteSharedMemory( &m_shmem_ptr, &m_shmem_size, m_shmem_name );
    deleteSemaphore( &m_sem_lock, m_sem_lock_name );
    deleteSemaphore( &m_sem_query, m_sem_query_name );
    deleteSemaphore( &m_sem_reply, m_sem_reply_name );
    deleteSemaphore( &m_sem_notify, m_sem_notify_name );

    std::cerr << "Done.\n";
    m_cleanup_pid = -1;
}



IPCController::~IPCController()
{
    std::cerr << __PRETTY_FUNCTION__ << "\n";
}

int
IPCController::run(int argc, char **argv)
{
    m_cleanup_pid = getpid();

    // --- Install signal handlers ---------------------------------------------
    struct sigaction act;
    memset( &act, 0, sizeof(act) );
    sigemptyset( &act.sa_mask );

    act.sa_handler = handle_SIGTERM;
    if( sigaction( SIGTERM, &act, NULL ) != 0 ) {
        std::cerr << "sigaction/SIGTERM failed: " << strerror( errno ) << "\n";
        m_job_state = TRELL_JOBSTATE_FAILED;
    }
    act.sa_handler = handle_SIGALRM;
    if( sigaction( SIGALRM, &act, NULL ) != 0 ) {
        std::cerr << "sigaction/SIGALRM failed: " << strerror( errno ) << "\n";
        m_job_state = TRELL_JOBSTATE_FAILED;
    }

    // --- Unblock signals we want ---------------------------------------------
    sigset_t mask;
    sigemptyset( &mask );
    sigaddset( &mask, SIGTERM );
    sigaddset( &mask, SIGALRM );
    if( sigprocmask( SIG_UNBLOCK, &mask, NULL ) != 0 ) {
        std::cerr << "sigprocmask failed: " << strerror( errno ) << "\n";
        m_job_state = TRELL_JOBSTATE_FAILED;
    }

    
    
   
 /*   
     if( atexit( at_exit ) != 0 ) {
        std::cerr << "Failed to set at exit function:" << strerror(errno) << "\n";
    }

*/
//    kill( getpid(), SIGUSR1 );


    for( int i=0; environ[i] != NULL; i++ ) {
        std::cerr << "ENV " << i <<": " << environ[i] << std::endl;
    }

    for( int i=0; i<argc; i++ ) {
        std::cerr << "ARG " << i << ": " << argv[i] << std::endl;
    }    

    // m_id = env[ TINIA_JOB_ID ].
    const char* tinia_job_id = getenv( "TINIA_JOB_ID" );
    if( tinia_job_id == NULL ) {
        std::cerr << "Environment variable 'TINIA_JOB_ID' not set, exiting.\n";
        m_job_state = TRELL_JOBSTATE_FAILED;
    }
    else {
        m_id = std::string( tinia_job_id );
    }

    if( m_id.empty() ) {
        std::cerr << "empty id, exiting.\n";
        m_job_state = TRELL_JOBSTATE_FAILED;
    }
    for( size_t i=0; i<m_id.size(); i++ ) {
        if( !(isalnum( m_id[i] ) || m_id[i] == '_') ) {
            std::cerr << "Illegal id '" << m_id << "', exiting.\n";
            m_job_state = TRELL_JOBSTATE_FAILED;
        }
    }

    // m_master_id = env[ TINIA_MASTER_ID ].
    const char* tinia_master_id = getenv( "TINIA_MASTER_ID" );
    if( tinia_master_id == NULL ) {
        std::cerr << "Environment variable 'TINIA_MASTER_ID' not set, exiting.\n";
        m_job_state = TRELL_JOBSTATE_FAILED;
    }
    else {
        m_master_id = std::string( tinia_master_id );
    }

    m_shmem_name      = "/" + m_id + "_shmem";
    m_sem_lock_name   = "/" + m_id + "_lock";
    m_sem_query_name  = "/" + m_id + "_query";
    m_sem_reply_name  = "/" + m_id + "_reply";
    m_sem_notify_name = "/" + m_id + "_notify";

    if( m_job_state == TRELL_JOBSTATE_NOT_STARTED ) {
        if( !createSharedMemory( &m_shmem_ptr, &m_shmem_size, m_shmem_name, m_shmem_size ) ||
            !createSemaphore( &m_sem_lock, m_sem_lock_name ) ||
            !createSemaphore( &m_sem_query, m_sem_query_name ) ||
            !createSemaphore( &m_sem_reply, m_sem_reply_name ) ||
            !createSemaphore( &m_sem_notify, m_sem_notify_name ) )
        {
            m_job_state = TRELL_JOBSTATE_FAILED;
        }
    }


    if( m_job_state == TRELL_JOBSTATE_NOT_STARTED && !m_is_master ) {
        messenger_status_t mrv = messenger_init( &m_master_mbox,
                                                  m_master_id.c_str(),
                                                  m_logger_callback,
                                                  m_logger_data );
        if( mrv != MESSENGER_OK ) {
            std::cerr << "Failed to connect to master: " << messenger_strerror( mrv ) << "\n";
            m_job_state = TRELL_JOBSTATE_FAILED;
        }
    }
    std::cerr << "Finished running setup code:\n";
    std::cerr << "  id               = " << m_id << "\n";
    std::cerr << "  pid              = " << getpid() << "\n";
    std::cerr << "  master id        = " << m_master_id << "\n";
    std::cerr << "  shared mem       = " << m_shmem_name << "\n";
    std::cerr << "  shared mem size  = " << m_shmem_size << " bytes\n";
    std::cerr << "  lock semaphore   = " << m_sem_lock_name << "\n";
    std::cerr << "  query semaphore  = " << m_sem_query_name << "\n";
    std::cerr << "  reply semaphore  = " << m_sem_reply_name << "\n";
    std::cerr << "  notify semaphore = " << m_sem_notify_name << "\n";



    if( m_job_state == TRELL_JOBSTATE_NOT_STARTED ) {
        if( !init() ) {
            m_job_state = TRELL_JOBSTATE_FAILED;
            std::cerr << "Init failed.\n";
        }
    }

    if( m_job_state == TRELL_JOBSTATE_NOT_STARTED ) {
        m_job_state = TRELL_JOBSTATE_RUNNING;
        sendHeartBeat();

        mainloop();
        
        // We exited for some reason.
        if( m_job_state == TRELL_JOBSTATE_FINISHED ) {
            m_job_state = TRELL_JOBSTATE_TERMINATED_SUCCESSFULLY;
        }
        else {
            m_job_state = TRELL_JOBSTATE_TERMINATED_UNSUCCESSFULLY;
        }
        sendHeartBeat();
    }
    failHard();
    return EXIT_SUCCESS;
}

void
IPCController::often()
{
    // does nothing.
}

void
IPCController::mainloop()
{
    // Open sem_lock for business
    sem_post( m_sem_lock );
    sendHeartBeat();

    timespec last_periodic;
    clock_gettime( CLOCK_REALTIME, &last_periodic );
    while( m_job_state == TRELL_JOBSTATE_RUNNING ) {
        if( exit_flag != 0 ) {
            m_job_state = TRELL_JOBSTATE_FAILED;
            std::cerr << "exit flag set\n";
        }
        
        // invoked for each iteration, to act on that signals have occured.        
        often();
        if( m_job_state != TRELL_JOBSTATE_RUNNING ) {
            break;
        }

        timespec timeout;
        clock_gettime( CLOCK_REALTIME, &timeout );

        // Check if it is time for a new periodic run.
        if( last_periodic.tv_sec+60 < timeout.tv_sec ) {
            last_periodic = timeout;
            if(!periodic()) {
                m_job_state = TRELL_JOBSTATE_FAILED;
                std::cerr << "Periodic failed.\n";
            }
            sendHeartBeat();
        }

        // Then check for messages or timeout within a minute whatever comes first
        timeout.tv_sec += 5;

        if( sem_timedwait( m_sem_query, &timeout ) == 0 ) {

            // Got a query message, sync memory into this process
            msync( m_shmem_ptr, m_shmem_size, MS_SYNC );

            // Act on the message
            size_t osize = handle( (trell_message*)m_shmem_ptr, m_shmem_size );

            // Sync memory out of this processes and notify about reply.
            msync( m_shmem_ptr, osize + TRELL_MSGHDR_SIZE, MS_SYNC | MS_INVALIDATE );
            sem_post( m_sem_reply );
        }

        if( m_notify ) {
            // Notify has been invoked while the lock has been locked, we do
            // the notification for them.
            m_notify = false;
            int waiting = 0;
            if( sem_getvalue( m_sem_notify, &waiting ) == 0 ) {
                for(int i=waiting; i<1; i++ ) {
                    sem_post( m_sem_notify );
                }
            }
        }

    }
    sendHeartBeat();

}


// --- send small message callbacks and invoker --------------------------------

struct send_small_message_data {
    TrellMessageType    m_query;
    TrellMessageType    m_reply;
};

static
int
send_small_message_producer( void*           data,
                             size_t*         bytes_written,
                             unsigned char*  buffer,
                             size_t          buffer_size )
{
    send_small_message_data* pd = reinterpret_cast<send_small_message_data*>( data );
    trell_message* msg = reinterpret_cast<trell_message*>( buffer );
    msg->m_type = pd->m_query;
    msg->m_size = 0;
    *bytes_written = TRELL_MSGHDR_SIZE;
    return 0;
}

static
int
send_small_message_consumer(  void* data,
                          unsigned char* buffer,
                          size_t offset,
                          size_t bytes,
                          int more )
{
    send_small_message_data* pd = reinterpret_cast<send_small_message_data*>( data );
    trell_message_t* msg = (trell_message_t*)buffer;
    pd->m_reply = msg->m_type;
    return 0;
}



TrellMessageType
IPCController::sendSmallMessage( const std::string& message_box_id, TrellMessageType query )
{
    send_small_message_data pd = { query, TRELL_MESSAGE_ERROR };
    switch( messenger_do_roundtrip( send_small_message_producer, &pd,
                                    send_small_message_consumer, &pd,
                                    m_logger_callback, m_logger_data,
                                    message_box_id.c_str(),
                                    0 ) )
    {
    case MESSENGER_OK:
        m_logger_callback( m_logger_data, 2, package.c_str(), "Successfully sent small message to '%s'.",
                           message_box_id.c_str() );
        return pd.m_reply;
    default:
        m_logger_callback( m_logger_data, 0, package.c_str(), "Failed to send small message to '%s'.",
                           message_box_id.c_str());
        return TRELL_MESSAGE_ERROR;
    }
}


// --- send heartbeat callbacks and invoker ------------------------------------
struct send_heartbeat_data {
    TrellJobState   m_state;    // to producer
    std::string&    m_jobid;    // to producer
    int             m_die;      // from consumer
};

static
int
send_heartbeat_producer( void*           data,
                         size_t*         bytes_written,
                         unsigned char*  buffer,
                         size_t          buffer_size )
{
    send_heartbeat_data* pd = reinterpret_cast<send_heartbeat_data*>( data );
    trell_message* msg = reinterpret_cast<trell_message*>( buffer );
    
    msg->m_type = TRELL_MESSAGE_HEARTBEAT;
    msg->m_size = offsetof(trell_message_t, m_ping_payload.m_tail ) - TRELL_MSGHDR_SIZE;
    msg->m_ping_payload.m_state = pd->m_state;
    strncpy( msg->m_ping_payload.m_job_id, pd->m_jobid.c_str(), TRELL_JOBID_MAXLENGTH );
    msg->m_ping_payload.m_job_id[ TRELL_JOBID_MAXLENGTH ] = '\0';

    *bytes_written = offsetof(trell_message_t, m_ping_payload.m_tail );
    return 0;
}

static
int
send_heartbeat_consumer(  void* data,
                          unsigned char* buffer,
                          size_t offset,
                          size_t bytes,
                          int more )
{
    send_heartbeat_data* pd = reinterpret_cast<send_heartbeat_data*>( data );
    trell_message_t* msg = (trell_message_t*)buffer;
    if( msg->m_type == TRELL_MESSAGE_OK ) {
        pd->m_die = 0;
        return 0;
    }
    else if( msg->m_type == TRELL_MESSAGE_DIE ) {
        pd->m_die = 1;
        return 0;
    }
    else {
        return -1;
    }    
}

bool
IPCController::sendHeartBeat()
{
    if( m_is_master ) {
        return true; // Don't send heartbeats to oneself.
    }
    
    send_heartbeat_data pd = { m_job_state, m_id, 0 };
    switch( messenger_do_roundtrip( send_heartbeat_producer, &pd,
                                    send_heartbeat_consumer, &pd,
                                    m_logger_callback, m_logger_data,
                                    m_master_id.c_str(),
                                    0 ) )
    {
    case MESSENGER_OK:
        m_logger_callback( m_logger_data, 2, package.c_str(), "Successfully sent heartbeat." );
        return true;
    default:
        m_logger_callback( m_logger_data, 0, package.c_str(), "Failed to send heartbeat." );
        return false;
    }
}




bool
IPCController::periodic()
{
    return true;
}


bool
IPCController::init()
{
    return true;
}

void
IPCController::cleanup()
{
}

}

}
