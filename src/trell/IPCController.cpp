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
#include "tinia/trell/messenger.h"

namespace tinia {
namespace trell {

namespace {
static std::list<IPCController*> instances;
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
    instances.push_back(this);
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

namespace {
static void cleanup_handler( int )
{
    for( auto it=instances.begin(); it!=instances.end(); ++it ) {
        (*it)->failHard();
    }
    exit( EXIT_SUCCESS );
}

static void at_exit()
{
    for( auto it=instances.begin(); it!=instances.end(); ++it ) {
        (*it)->failHard();
    }
}

//static void alarm_handler( int sig )
//{
//    std::cerr << "alarm handler.\n";
//}

static void usr1_handler( int sig )
{
    std::cerr << "usr1_handler.\n";

//    signal( SIGUSR1, usr1_handler );
}
}



IPCController::~IPCController()
{
    std::cerr << __PRETTY_FUNCTION__ << "\n";
}

int
IPCController::run(int argc, char **argv)
{
    m_cleanup_pid = getpid();
    // argv[0] - exe
    // argv[1] - job name
    // argv[2] - master id

    if( argc != 3 ) {
        std::cerr << "argc != 3, exiting.\n";
        return EXIT_FAILURE;
    }

    // Install signal handlers, trying to maximize the likelyhood of a decent
    // cleanup
    if( signal( SIGINT, cleanup_handler ) == SIG_ERR ) {
        std::cerr << "Failed to set SIGINT handler:" << strerror(errno) << "\n";
    }

    if( atexit( at_exit ) != 0 ) {
        std::cerr << "Failed to set at exit function:" << strerror(errno) << "\n";
    }


/*    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = 0;
    if( sigaction( SIGALRM, &sa, NULL ) == -1 ) {
        std::cerr << "Failed to set alarm handler: " << strerror(errno) << "\n";
    }
    */
    if( sigset( SIGALRM, usr1_handler ) == SIG_ERR ) {
        std::cerr << "Failed to set alarm handler: " << strerror(errno) << "\n";
    }
    std::cerr << "Set alarm handler\n";

//    kill( getpid(), SIGUSR1 );



    m_id = std::string( argv[1] );
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

    m_master_id = std::string( argv[2] );
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
        messenger_status_t mrv = messenger_init( &m_master_mbox, m_master_id.c_str() );
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

    timespec last_periodic;
    clock_gettime( CLOCK_REALTIME, &last_periodic );

    std::string xml;
    if( !m_is_master && m_job_state == TRELL_JOBSTATE_NOT_STARTED ) {
        messenger_status_t mrv;

        mrv = messenger_lock( &m_master_mbox );
        if( mrv != MESSENGER_OK ) {
            std::cerr << __func__ << ": messenger_lock(master): " << messenger_strerror( mrv ) << "\n";
        }
        else {
            size_t msg_size =(m_id.length()+1u) + offsetof( trell_message, m_args_payload );
            if( msg_size < m_master_mbox.m_shmem_size ) {
                trell_message* msg = reinterpret_cast<trell_message*>( m_master_mbox.m_shmem_ptr );

                msg->m_type = TRELL_MESSAGE_ARGS;
                msg->m_size = msg_size - TRELL_MSGHDR_SIZE;
                strcpy( msg->m_args_payload.m_job_id, m_id.c_str() );

                mrv = messenger_post( &m_master_mbox, TRELL_MSGHDR_SIZE );
                if( mrv != MESSENGER_OK ) {
                    std::cerr << __func__ << ": messenger_post(master): " << messenger_strerror( mrv ) << "\n";
                }
                else {
                    if( msg->m_type == TRELL_MESSAGE_XML ) {
                        xml = std::string( msg->m_xml_payload, msg->m_xml_payload+msg->m_size );
                    }
                }
            }
            mrv = messenger_unlock( &m_master_mbox );
            if( mrv != MESSENGER_OK ) {
                std::cerr << __func__ << ": messenger_unlock(master): " << messenger_strerror( mrv ) << "\n";
            }
        }

        if( xml.empty() ) {
            m_job_state = TRELL_JOBSTATE_FAILED;
            std::cerr << "Failed to get arguments.\n";
        }
    }


    if( m_job_state == TRELL_JOBSTATE_NOT_STARTED ) {
        if( !init(xml) ) {
            m_job_state = TRELL_JOBSTATE_FAILED;
            std::cerr << "Init failed.\n";
        }
    }

    if( m_job_state == TRELL_JOBSTATE_NOT_STARTED ) {
        m_job_state = TRELL_JOBSTATE_RUNNING;
        sendHeartBeat();

        sem_post( m_sem_lock );
        while( m_job_state == TRELL_JOBSTATE_RUNNING ) {

            timespec timeout;
            clock_gettime( CLOCK_REALTIME, &timeout );


            // Check if it is time for a new periodic run.
            if( last_periodic.tv_sec+60 < timeout.tv_sec ) {
                last_periodic = timeout;
                if(!periodic()) {
                    m_job_state = TRELL_JOBSTATE_FAILED;
                    std::cerr << "Periodic failed.\n";
                }
            }
            sendHeartBeat();

            // Then check for messages or timeout within a minute whatever comes first
            timeout.tv_sec += 5;

            if( sem_timedwait( m_sem_query, &timeout ) == 0 ) {
                std::cerr << "Got message.\n";
                msync( m_shmem_ptr, m_shmem_size, MS_SYNC );
                size_t osize = handle( (trell_message*)m_shmem_ptr, m_shmem_size );
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




TrellMessageType
IPCController::sendSmallMessage( const std::string& message_box_id, TrellMessageType query )
{
    TrellMessageType r = TRELL_MESSAGE_ERROR;

    messenger m;
    messenger_status_t mrv;
    mrv = messenger_init( &m, message_box_id.c_str() );
    if( mrv != MESSENGER_OK ) {
        std::cerr << __func__ << ": messenger_init("<<message_box_id<<"): " << messenger_strerror( mrv ) << "\n";
    }
    else {

        mrv = messenger_lock( &m );
        if( mrv != MESSENGER_OK ) {
            std::cerr << __func__ << ": messenger_lock("<<message_box_id<<"): " << messenger_strerror( mrv ) << "\n";
        }
        else {
            trell_message* msg = reinterpret_cast<trell_message*>( m.m_shmem_ptr );
            msg->m_type = query;
            msg->m_size = 0u;

            mrv = messenger_post( &m, TRELL_MSGHDR_SIZE );
            if( mrv != MESSENGER_OK ) {
                std::cerr << __func__ << ": messenger_post("<<message_box_id<<"): " << messenger_strerror( mrv ) << "\n";
            }
            else {
                r = (TrellMessageType)msg->m_type;
            }

            mrv = messenger_unlock( &m );
            if( mrv != MESSENGER_OK ) {
                std::cerr << __func__ << ": messenger_unlock("<<message_box_id<<"): " << messenger_strerror( mrv ) << "\n";
            }
        }
        mrv = messenger_free( &m );
        if( mrv != MESSENGER_OK ) {
            std::cerr << __func__ << ": messenger_free("<<message_box_id<<"): " << messenger_strerror( mrv ) << "\n";
        }
    }
    return r;
}

bool
IPCController::sendHeartBeat()
{
    if( m_is_master ) {
        return true; // Don't send heartbeats to oneself.
    }
    messenger_status_t mrv;

    mrv = messenger_lock( &m_master_mbox );
    if( mrv != MESSENGER_OK ) {
        std::cerr << __func__ << ": messenger_lock(master): " << messenger_strerror( mrv ) << "\n";
        return false;
    }


    // Send ping message
    size_t msg_size =(m_id.length()+1u) + offsetof( trell_message, m_ping_payload.m_job_id );
    if( m_master_mbox.m_shmem_size <=  msg_size ) {
        std::cerr << "Insufficient size of shared memory.\n";
        mrv = messenger_unlock( &m_master_mbox );
        if( mrv != MESSENGER_OK ) {
            std::cerr << __func__ << ": messenger_unlock(master): " << messenger_strerror( mrv ) << "\n";
        }
        return false;
    }

    trell_message* msg = reinterpret_cast<trell_message*>( m_master_mbox.m_shmem_ptr );

    msg->m_type = TRELL_MESSAGE_HEARTBEAT;
    msg->m_size = msg_size - TRELL_MSGHDR_SIZE;
    msg->m_ping_payload.m_state = m_job_state;
    strcpy( msg->m_ping_payload.m_job_id, m_id.c_str() );

    bool retval = true;

    mrv = messenger_post( &m_master_mbox, msg_size );
    if( mrv != MESSENGER_OK ) {
        std::cerr << __func__ << ": messenger_post(master): " << messenger_strerror( mrv ) << "\n";
        retval = false;
    }
    else {
        switch( msg->m_type ) {
        case TRELL_MESSAGE_OK:
            break;
        case TRELL_MESSAGE_DIE:
            m_job_state = TRELL_JOBSTATE_FAILED;
            break;
        default:
            std::cerr << __func__ << ": unexpected message: " << msg->m_type << "\n";
        }
    }

    mrv = messenger_unlock( &m_master_mbox );
    if( mrv != MESSENGER_OK ) {
        std::cerr << __func__ << ": messenger_unlock(master): " << messenger_strerror( mrv ) << "\n";
        retval = false;
    }
    return retval;
}




bool
IPCController::periodic()
{
    return true;
}


bool
IPCController::init( const std::string& xml )
{
    return true;
}

void
IPCController::cleanup()
{
}

}

}
