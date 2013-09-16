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

#if 0
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


#endif
}



IPCController::IPCController( bool is_master )
    : m_ipc_pid( getpid() ),
      m_cleanup_pid( -1 ),
      m_msgbox( NULL ),
      m_is_master( is_master ),
      m_job_state( TRELL_JOBSTATE_NOT_STARTED )
{
    //instances.push_back(this);
}

void
IPCController::finish()
{
    m_job_state = TRELL_JOBSTATE_FINISHED;
    messenger_server_break_mainloop( m_msgbox );
}

void
IPCController::fail()
{
    m_job_state = TRELL_JOBSTATE_FAILED;
    messenger_server_break_mainloop( m_msgbox );
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
    messenger_server_notify( m_msgbox );
#if 0    
    int value;
    if( sem_trywait( m_msgbox.m_sem_lock ) == 0 ) {
        if( sem_getvalue( m_msgbox.m_sem_notify, &value ) == 0 ) {
            for( int i=value; i<1; i++) {
                sem_post( m_msgbox.m_sem_notify );
            }
        }
        sem_post( m_msgbox.m_sem_lock );
    }
    else {
        // we didn't get the lock, we let the IPC thread do the notification
        // when it is finished with the request
        m_msgbox.m_notify = true;
    }
#endif
}

void IPCController::addScript(const std::string &script)
{
    m_scripts.push_back(script);
}

bool IPCController::onGetScripts(size_t &result_size, char *buffer, const size_t buffer_size)
{
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

    messenger_server_destroy( m_msgbox );

    std::cerr << "Done.\n";
    m_cleanup_pid = -1;
}



IPCController::~IPCController()
{
    std::cerr << __PRETTY_FUNCTION__ << "\n";
}


messenger_status_t
IPCController::message_consumer( void*                     data,
                  const char*               buffer,
                  const size_t              buffer_bytes_,
                  const int                 first,
                  const int                 more ) 
{
    IPCController::Context* ctx = reinterpret_cast<IPCController::Context*>( data );
    if( first ) {
        ctx->m_buffer_offset = 0;
    }
    
    trell_message* msg = (trell_message*)buffer;
    size_t buffer_bytes = msg->m_size + TRELL_MSGHDR_SIZE;
    
    if( ctx->m_buffer_size <= ctx->m_buffer_offset + buffer_bytes ) {
        return MESSENGER_ERROR;
    }
    memcpy( ctx->m_buffer + ctx->m_buffer_offset, buffer, buffer_bytes );
    ctx->m_buffer_offset += buffer_bytes;
    
    if( !more ) {
        ctx->m_output_bytes = ctx->m_ipc_controller->handle( reinterpret_cast<trell_message_t*>( ctx->m_buffer ),
                                                              ctx->m_buffer_size );
    }
    return MESSENGER_OK;
}

messenger_status_t
IPCController::message_producer( void*         data,
                  int*          more,
                  char*         buffer,
                  size_t*       buffer_bytes,
                  const size_t  buffer_size,
                  const int     first )
{
    IPCController::Context* ctx = reinterpret_cast<IPCController::Context*>( data );
    if( first ) {
        ctx->m_buffer_offset = 0;
    }
    size_t bytes = ctx->m_output_bytes - ctx->m_buffer_offset;
    if( buffer_size < bytes ) {
        bytes = buffer_size;
    }
    memcpy( buffer, ctx->m_buffer + ctx->m_buffer_offset, bytes );
    ctx->m_buffer_offset += bytes;
    *buffer_bytes = bytes;
    *more = ctx->m_buffer_offset < ctx->m_output_bytes ? 1 : 0;
    return MESSENGER_OK;
}

messenger_status_t
IPCController::handle_periodic( void* data, int seconds )
{
    IPCController::Context* ctx = reinterpret_cast<IPCController::Context*>( data );
    if( ctx->m_ipc_controller->periodic() ) {
        return MESSENGER_OK;
    }
    else {
        return MESSENGER_ERROR;
    }
}


int
IPCController::run(int argc, char **argv)
{
    m_cleanup_pid = getpid();
#if 0

    
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

#endif
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


    if( m_job_state == TRELL_JOBSTATE_NOT_STARTED ) {
        m_msgbox = new messenger_server_t;
        if( messenger_server_create( m_msgbox, m_id.c_str(), m_logger_callback, m_logger_data ) != MESSENGER_OK ) {
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
    std::cerr << "  master id        = " << m_master_id << "\n";



    if( m_job_state == TRELL_JOBSTATE_NOT_STARTED ) {
        if( !init() ) {
            m_job_state = TRELL_JOBSTATE_FAILED;
            std::cerr << "Init failed.\n";
        }
    }

    if( m_job_state == TRELL_JOBSTATE_NOT_STARTED ) {
        m_job_state = TRELL_JOBSTATE_RUNNING;
        sendHeartBeat();


        Context ctx;
        ctx.m_ipc_controller = this;
        ctx.m_buffer_size = 1000*1024*1024;
        ctx.m_buffer = new char[ctx.m_buffer_size];
    
        if( messenger_server_mainloop( m_msgbox,
                                       message_consumer, &ctx,
                                       message_producer, &ctx,
                                       handle_periodic, &ctx ) == MESSENGER_OK )
        {
            m_job_state = TRELL_JOBSTATE_TERMINATED_SUCCESSFULLY;
        }
        else {
            m_job_state = TRELL_JOBSTATE_TERMINATED_UNSUCCESSFULLY;
        }
         
        delete ctx.m_buffer;        
#if 0        
//        mainloop();
        
        // We exited for some reason.
        if( m_job_state == TRELL_JOBSTATE_FINISHED ) {
            m_job_state = TRELL_JOBSTATE_TERMINATED_SUCCESSFULLY;
        }
        else {
            m_job_state = TRELL_JOBSTATE_TERMINATED_UNSUCCESSFULLY;
        }
#endif
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
#if 0
    // Open sem_lock for business
    sem_post( m_msgbox.m_sem_lock );
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

        if( sem_timedwait( m_msgbox.m_sem_query, &timeout ) == 0 ) {

            // Got a query message, sync memory into this process
            msync( m_msgbox.m_shmem_ptr, m_msgbox.m_shmem_size, MS_SYNC );

            // Act on the message
            size_t osize = handle( (trell_message*)m_msgbox.m_shmem_ptr, m_msgbox.m_shmem_size );

            // Sync memory out of this processes and notify about reply.
            msync( m_msgbox.m_shmem_ptr, osize + TRELL_MSGHDR_SIZE, MS_SYNC | MS_INVALIDATE );
            sem_post( m_msgbox.m_sem_reply );
        }

        if( m_msgbox.m_notify ) {
            // Notify has been invoked while the lock has been locked, we do
            // the notification for them.
            m_msgbox.m_notify = false;
            int waiting = 0;
            if( sem_getvalue( m_msgbox.m_sem_notify, &waiting ) == 0 ) {
                for(int i=waiting; i<1; i++ ) {
                    sem_post( m_msgbox.m_sem_notify );
                }
            }
        }

    }
    sendHeartBeat();
#endif
}





TrellMessageType
IPCController::sendSmallMessage( const std::string& message_box_id, TrellMessageType query )
{
    tinia_msg_t q;
    q.type = query;
    q.size = 0;
    
    tinia_msg_t r;
    size_t rs;
    messenger_status_t rv = messenger_do_roundtrip( &q, sizeof(q), &r, &rs, sizeof(r),
                                                    m_logger_callback, m_logger_data,
                                                    message_box_id.c_str(),
                                                    0 );

    if( (rv == MESSENGER_OK) && (rs == sizeof(r)) ) {
        m_logger_callback( m_logger_data, 2, package.c_str(), "Successfully sent small message to '%s'.",
                           message_box_id.c_str() );
        return TRELL_MESSAGE_ERROR;
    }
    else {
        m_logger_callback( m_logger_data, 0, package.c_str(), "Failed to send small message to '%s'.",
                           message_box_id.c_str());
        return r.type;
    }
}


bool
IPCController::sendHeartBeat()
{
    if( m_is_master ) {
        return true; // Don't send heartbeats to oneself.
    }

    tinia_msg_heartbeat_t query;
    query.msg.type = TRELL_MESSAGE_HEARTBEAT;
    query.msg.size = sizeof(query)-TRELL_MSGHDR_SIZE;
    query.state = m_job_state;
    strncpy( query.job_id, m_id.c_str(), TRELL_JOBID_MAXLENGTH );
    query.job_id[ TRELL_JOBID_MAXLENGTH ] = '\0';
    
    tinia_msg_t reply;
    size_t reply_actual;

    messenger_status_t rv = messenger_do_roundtrip( &query, sizeof(query),
                                                    &reply, &reply_actual, sizeof(reply),
                                                    m_logger_callback, m_logger_data,
                                                    m_master_id.c_str(),
                                                    0 );

    if( (rv == MESSENGER_OK) && (reply_actual == sizeof(reply)) && (reply.type == TRELL_MESSAGE_OK ) ) {
        m_logger_callback( m_logger_data, 2, package.c_str(), "Successfully sent heartbeat." );
        return true;
    }
    else {
        m_logger_callback( m_logger_data, 0, package.c_str(), "Failed to send heartbeat." );
        return false;
    }

}

bool
IPCController::periodic()
{
    sendHeartBeat();
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
