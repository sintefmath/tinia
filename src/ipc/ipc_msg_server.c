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
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <pthread.h>
#include <string.h> // memset + strerror
#include <time.h>

#include <errno.h>

// shmem stuff
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include <tinia/ipc/ipc_msg.h>
#include "ipc_msg_internal.h"


tinia_ipc_msg_server_t*
ipc_msg_server_create(const char*       jobid,
                       tinia_ipc_msg_log_func_t logger_f,
                       void*            logger_d  )
{
    static const char* who = "tinia.ipc.msg.server.create";
    char errnobuf[256];
    int rc;
            
    // block SIGTERM (we will grab it in mainloop)
    sigset_t signal_mask;
    sigemptyset (&signal_mask);
    sigaddset (&signal_mask, SIGTERM);
    rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
    if( rc != 0 ) {
        logger_f( logger_d, 0, who,
                  "pthread_sigmask failed: %s",
                  ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
        return NULL;
    }
    
    // -------------------------------------------------------------------------
    // --- first, do stuff that doesn't requere server structure ---------------
    // -------------------------------------------------------------------------

    // --- create name of shared memory ----------------------------------------
    char path[256];
    if( ipc_msg_shmem_path( logger_f,
                            logger_d,
                            path,
                            sizeof(path),
                            jobid ) != 0 )
    {
        return NULL;
    }

    // --- get page size, used to pad header size ------------------------------    
    long int page_size = sysconf( _SC_PAGESIZE );
    if( page_size == -1 ) {
        logger_f( logger_d, 0, who,
                  "sysconf( _SC_PAGESIZE ) failed: %s",
                  ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
        return NULL;
    }    

    // -------------------------------------------------------------------------
    // --- allocate structure and begin initialization -------------------------
    // -------------------------------------------------------------------------
    tinia_ipc_msg_server_t* server = (tinia_ipc_msg_server_t*)malloc( sizeof(tinia_ipc_msg_server_t) );
    server->logger_f = logger_f;
    server->logger_d = logger_d;
    server->shmem_name = strdup( path );
    server->shmem_base = MAP_FAILED;
    server->shmem_total_size = 0;
    server->shmem_header_ptr = (tinia_ipc_msg_header_t*)MAP_FAILED;
    server->shmem_header_size = ((sizeof(tinia_ipc_msg_header_t)+page_size-1)/page_size)*page_size; // multiple of page size
    server->shmem_payload_ptr = MAP_FAILED;
    server->shmem_payload_size = 0;
    
    size_t requested_payload_size = 8*1024*1024;   //TINIA_IPC_MSG_PART_MIN_BYTES;
    if( ipc_msg_fake_shmem != 0 ) {
        
        rc = pthread_mutex_lock( &ipc_msg_fake_shmem_lock );
        if( rc != 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "pthread_mutex_lock( ipc_msg_fake_shmem_lock ) failed: %s",
                              ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
            return NULL;
        }
        else {
            
            ipc_msg_fake_shmem_size = ((requested_payload_size+server->shmem_header_size+page_size-1)/page_size)*page_size;
            ipc_msg_fake_shmem_ptr = malloc( ipc_msg_fake_shmem_size );
            if( ipc_msg_fake_shmem_ptr == NULL ) {
                server->logger_f( server->logger_d, 0, who,
                                  "malloc( ipc_msg_fake_shmem_size ) failed." );
                pthread_mutex_unlock( &ipc_msg_fake_shmem_lock );
                return NULL;
            }
            server->shmem_total_size = ipc_msg_fake_shmem_size;
            server->shmem_payload_size = server->shmem_total_size - server->shmem_header_size;
            server->shmem_base = ipc_msg_fake_shmem_ptr;
            ipc_msg_fake_shmem_users++;

            rc = pthread_mutex_unlock( &ipc_msg_fake_shmem_lock );
            if( rc != 0 ) {
                server->logger_f( server->logger_d, 0, who,
                                  "pthread_mutex_unlock( ipc_msg_fake_shmem_lock ) failed: %s",
                                  ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
            }
        }        
    }
    else {
        // --- remove old shared memory segment if it hasn't been cleaned up -------
        if( shm_unlink( path ) == 0 ) {
            server->logger_f( server->logger_d, 2, who,
                              "Removed existing shared memory segment '%s'\n",
                              path );
        }
        
        // --- create and open -----------------------------------------------------
        // NOTE: mode 0600, only user can communicate. Change if necessary.
        int fd = shm_open( server->shmem_name, O_RDWR | O_CREAT | O_EXCL, 0600 );
        if( fd < 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "Failed to create shared memory: %s",
                              ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
            ipc_msg_server_delete( server );
            return NULL;
        }
        
        // --- set size of shared memory segment -----------------------------------
        if( ftruncate( fd, requested_payload_size+server->shmem_header_size ) != 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "Failed to set shared memory size to %ld: %s",
                              (requested_payload_size+server->shmem_header_size),
                              ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
            close( fd );
            ipc_msg_server_delete( server );
            return NULL;
        }
        
        // --- query actual size of shared memory ----------------------------------
        struct stat fstat_buf;
        if( fstat( fd, &fstat_buf ) != 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "Failed to get shared memory size: %s",
                              ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
            close( fd );
            ipc_msg_server_delete( server );
            return NULL;
        }
        server->shmem_total_size = fstat_buf.st_size;
        if( server->shmem_total_size < requested_payload_size+server->shmem_header_size ) {
            // shouldn't happen
            server->logger_f( server->logger_d, 0, who,
                              "shmem size is less than requested!" );
            close( fd );
            ipc_msg_server_delete( server );
            return NULL;
        }
        server->shmem_payload_size = server->shmem_total_size - server->shmem_header_size;
        
        // --- map memory into address space ---------------------------------------
        server->shmem_base = mmap( NULL,
                                   server->shmem_total_size,
                                   PROT_READ | PROT_WRITE,
                                   MAP_SHARED,
                                   fd, 0 );
        if( server->shmem_base == MAP_FAILED ) {
            server->logger_f( server->logger_d, 0, who,
                              "Failed to map shared memory: %s",
                              ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
            close( fd );
            ipc_msg_server_delete( server );
            return NULL;
        }
        close( fd );
    }
    
    server->shmem_header_ptr = (tinia_ipc_msg_header_t*)server->shmem_base;
    server->shmem_payload_ptr = (char*)server->shmem_base + server->shmem_header_size;
    
    server->shmem_header_ptr->header_size = server->shmem_header_size;
    server->shmem_header_ptr->payload_size = server->shmem_payload_size;
    
    //fprintf( stderr, "I: %s: header=%lu bytes, payload=%lu bytes, total=%lu bytes.\n",
    //         server->shmem_name,
    //         server->shmem_header_size, server->shmem_payload_size, server->shmem_total_size );

    // -------------------------------------------------------------------------
    // --- shared memory is set up and mapped, set up pthreads-stuff -----------
    // -------------------------------------------------------------------------
    
    server->shmem_header_ptr->initialized = 0;
    server->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;

    pthread_mutexattr_t mutexattr;
    pthread_condattr_t condattr;

    server->thread_id = pthread_self();
    
    int failed = 0;

#define CHECK(A) \
do { \
    if(failed==0) { \
        if((rc=A)!=0) { \
            server->logger_f( server->logger_d, 0, who, "%s: %s",\
                              #A, ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) ); \
            failed=1; \
        } \
    } \
} while(0)
    
    // --- initialize transaction lock -----------------------------------------
    CHECK( pthread_mutexattr_init( &mutexattr ) );
    CHECK( pthread_mutexattr_setpshared( &mutexattr, PTHREAD_PROCESS_SHARED ) );
#ifdef DEBUG
    CHECK( pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_ERRORCHECK ) );
#endif
    CHECK( pthread_mutex_init( &server->shmem_header_ptr->transaction_lock, &mutexattr ) );
    CHECK( pthread_mutexattr_destroy( &mutexattr ) );

    // --- initialize operation lock -----------------------------------------------
    CHECK( pthread_mutexattr_init( &mutexattr ) );
    CHECK( pthread_mutexattr_setpshared( &mutexattr, PTHREAD_PROCESS_SHARED ) );
#ifdef DEBUG
    CHECK( pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_ERRORCHECK ) );
#endif
    CHECK( pthread_mutex_init( &server->shmem_header_ptr->operation_lock, &mutexattr ) );
    CHECK( pthread_mutexattr_destroy( &mutexattr ) );

    // --- initialize notification condition variable --------------------------
    CHECK( pthread_condattr_init( &condattr ) );
    CHECK( pthread_condattr_setpshared( &condattr, PTHREAD_PROCESS_SHARED ) );
    CHECK( pthread_cond_init( &server->shmem_header_ptr->notification_event, &condattr ) );
    CHECK( pthread_condattr_destroy( &condattr ) );

    // --- initialize server wakeup condition variable -------------------------
    CHECK( pthread_condattr_init( &condattr ) );
    CHECK( pthread_condattr_setpshared( &condattr, PTHREAD_PROCESS_SHARED ) );
    CHECK( pthread_cond_init( &server->shmem_header_ptr->server_event, &condattr ) );
    CHECK( pthread_condattr_destroy( &condattr ) );

    // --- initialize client wakeup condition variable -------------------------
    CHECK( pthread_condattr_init( &condattr ) );
    CHECK( pthread_condattr_setpshared( &condattr, PTHREAD_PROCESS_SHARED ) );
    CHECK( pthread_cond_init( &server->shmem_header_ptr->client_event, &condattr ) );
    CHECK( pthread_condattr_destroy( &condattr ) );
#undef CHECK
    
    // --- if some of the pthread-init-stuff has failed, give up ---------------
    if( failed != 0 ) {
        ipc_msg_server_delete( server );
        return NULL;
    }

    server->shmem_header_ptr->initialized = 1;
    return server;
}

int
ipc_msg_server_wipe( tinia_ipc_msg_log_func_t log_f,
                     void *log_d,
                     const char* jobid )
{
    char path[256];
    if( ipc_msg_shmem_path( log_f, log_d, path, sizeof(path), jobid ) == 0 ) {
        if( shm_unlink( path ) == 0 ) {
            return 0;
        }
    }
    return -1;
}

int
ipc_msg_server_delete( tinia_ipc_msg_server_t* server )
{
    static const char* who = "tinia.ipc.msg.server.delete";
    char errnobuf[256];

    int rc;
    if( server == NULL ) {
        return -1;
    }
    
    if( (server->shmem_header_ptr != (tinia_ipc_msg_header_t*)MAP_FAILED)
            && (server->shmem_header_ptr->initialized == 1 ) )
    {
#define CHECK(A) \
do { \
    rc=A; \
    if(rc!=0) { \
        server->logger_f( server->logger_d, 0, who, "%s: %s", #A, \
                          ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) ); \
    } \
} while(0)
        CHECK( pthread_cond_destroy( &server->shmem_header_ptr->client_event ) );
        CHECK( pthread_cond_destroy( &server->shmem_header_ptr->server_event ) );
        CHECK( pthread_cond_destroy( &server->shmem_header_ptr->notification_event ) );
        CHECK( pthread_mutex_destroy( &server->shmem_header_ptr->operation_lock ) );
        CHECK( pthread_mutex_destroy( &server->shmem_header_ptr->transaction_lock ) );
#undef CHECK
        server->shmem_header_ptr->initialized = 0;
    }

    if( ipc_msg_fake_shmem ) {
        rc = pthread_mutex_lock( &ipc_msg_fake_shmem_lock );
        if( rc != 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "pthread_mutex_lock( ipc_msg_fake_shmem_lock ) failed: %s",
                              ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
        }
        else {
            ipc_msg_fake_shmem_users--;
            if( server->shmem_base != NULL ) {
                free( server->shmem_base );
            }
            server->shmem_base = MAP_FAILED;
            ipc_msg_fake_shmem_ptr = NULL;
            ipc_msg_fake_shmem_size = 0;
            
            rc = pthread_mutex_unlock( &ipc_msg_fake_shmem_lock );
            if( rc != 0 ) {
                server->logger_f( server->logger_d, 0, who,
                                  "pthread_mutex_unlock( ipc_msg_fake_shmem_lock ) failed: %s",
                                  ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
            }
        }
    }
    else {
        if( server->shmem_base != MAP_FAILED ) {
            if( munmap( server->shmem_base, server->shmem_total_size ) != 0 ) {
                server->logger_f( server->logger_d, 0, who,
                                  "Failed to unmap shared memory failed: %s",
                                  ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
            }
            server->shmem_base = MAP_FAILED;
        }
        
        if( server->shmem_name != NULL ) {
            if( shm_unlink( server->shmem_name ) != 0 ) {
                server->logger_f( server->logger_d, 0, who,
                                  "Failed to unlink shared memory: %s",
                                  ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
            }
        }
    }
    if( server->shmem_name != NULL ) {
        free( server->shmem_name );
        server->shmem_name = NULL;
    }
        
    free( server );
    return 0;
}

int
ipc_msg_server_signal_client( char* errnobuf,
                              size_t errnobuf_size,
                              tinia_ipc_msg_server_t* server )
{
    static const char* who = "tinia.ipc.msg.server.signal.client";
    server->shmem_header_ptr->client_event_predicate = 1;
    int rc = pthread_cond_signal( &server->shmem_header_ptr->client_event );
    if( rc != 0 ) {
        server->logger_f( server->logger_d, 0, who,
                          "pthread_cond_signal( client_event ): %s",
                          ipc_msg_strerror_wrap(rc, errnobuf, errnobuf_size ) );
        server->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
        return -2;
    }
    return 0;
}

int
ipc_msg_server_wait_client( char* errnobuf,
                            size_t errnobuf_size,
                            tinia_ipc_msg_server_t* server )
{
    static const char* who = "tinia.ipc.msg.server.wait.client";

    struct timespec timeout;
    clock_gettime( CLOCK_REALTIME, &timeout );
    timeout.tv_sec += 1;
    
    server->shmem_header_ptr->server_event_predicate = 0;
    do {
        int rc = pthread_cond_timedwait( &server->shmem_header_ptr->server_event,
                                         &server->shmem_header_ptr->operation_lock,
                                         &timeout );
        if( rc != 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "pthread_cond_timedwait( server_event ): %s",
                              ipc_msg_strerror_wrap(rc, errnobuf, errnobuf_size ) );
            if( rc == ETIMEDOUT ) {
                return -1;
            }
            else {
                return -2;
            }
        }
    } while( server->shmem_header_ptr->server_event_predicate == 0 );
    return 0;
}


int
ipc_msg_server_recv( char* errnobuf,
                     size_t errnobuf_size,
                     tinia_ipc_msg_server_t* server,
                     tinia_ipc_msg_input_handler_func_t input_handler,
                     void* input_handler_data )
{
    static const char* who = "tinia.ipc.msg.server.recv";
    int ret = 0, part;

    tinia_ipc_msg_consumer_func_t consumer = NULL;
    void* consumer_data = NULL;
    
    // entry invariants:
    // - client has transaction lock
    // - server event has been notified (caught by caller)
    // - we have operation lock (from notification).
    
    struct timespec timeout;
    clock_gettime( CLOCK_REALTIME, &timeout );
    timeout.tv_sec += 5;

    for(part=0; ret==0; part++) {
        
        // --- check that state is as expected (and not error) -----------------
        if( server->shmem_header_ptr->state != IPC_MSG_STATE_CLIENT_TO_SERVER ) {
            server->logger_f( server->logger_d, 0, who,
                              "Got state %d, expected state %d.",
                              server->shmem_header_ptr->state,
                              IPC_MSG_STATE_CLIENT_TO_SERVER );
            ret = -1;
            server->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
            ipc_msg_server_signal_client( errnobuf, errnobuf_size, server );
            break;
        }

        // --- check that we have the correct message part ---------------------
        if( server->shmem_header_ptr->part != part ) {
            server->logger_f( server->logger_d, 0, who,
                              "Got part %d, expected part %d.",
                              server->shmem_header_ptr->part, part );
            ret = -1;
            server->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
            ipc_msg_server_signal_client( errnobuf, errnobuf_size, server );
            break;
        }
        
        // --- In first part, we determine the consumer -----------------------
        if( part==0 ) { // If first part, determine consumer
           
            if( input_handler( &consumer, &consumer_data,
                               input_handler_data,
                               (char*)server->shmem_payload_ptr,
                               server->shmem_header_ptr->bytes ) != 0 )
            {
                server->logger_f( server->logger_d, 0, who,
                                  "Input handler failed." );
                ret = -1;   // input handler failed, give up.
                server->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
                ipc_msg_server_signal_client( errnobuf, errnobuf_size, server );
                break;
            }
            if( consumer == NULL ) {
                server->logger_f( server->logger_d, 0, who,
                                  "Input handler returned nullptr." );
                ret = -2;   // this is really bad
                server->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
                ipc_msg_server_signal_client( errnobuf, errnobuf_size, server );
                break;
            }
        }
        
        int more = server->shmem_header_ptr->more;
        if( consumer( consumer_data,
                      (char*)server->shmem_payload_ptr,
                      server->shmem_header_ptr->bytes,
                      part,
                      server->shmem_header_ptr->more ) != 0 )
        {
            ret = -1;
            server->logger_f( server->logger_d, 0, who, "Consumer failed." );
            server->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
            ipc_msg_server_signal_client( errnobuf, errnobuf_size, server );
            break;
        }
        
        // --- if this was the last part, break out ----------------------------
        if( more == 0 ) {
            break;
        }
        // --- signal client for the next part ---------------------------------
        if( (ret = ipc_msg_server_signal_client( errnobuf, errnobuf_size, server )) != 0 ) {
            break;
        }
        // --- and wait until we have the next part ----------------------------
        if( (ret = ipc_msg_server_wait_client( errnobuf, errnobuf_size, server ) ) != 0 ) {
            break;
        }
    }
    return ret;
}

int
ipc_msg_server_send( char* errnobuf,
                     size_t errnobuf_size,
                     tinia_ipc_msg_server_t* server,
                     tinia_ipc_msg_output_handler_func_t output_handler,
                     void* output_handler_data )
{
    static const char* who = "tinia.ipc.msg.server.send";
    
    int ret = 0;

    tinia_ipc_msg_producer_func_t producer = NULL;
    void* producer_data = NULL;

    struct timespec timeout;
    clock_gettime( CLOCK_REALTIME, &timeout );
    timeout.tv_sec += 5;

    
    // --- send train of reply messages ----------------------------------------
    int part, more;
    for( part=0, more=1; (more==1) && (ret==0); part++ ) {
       
        // --- In first part, we determine the producer ------------------------
        if( part == 0 ) { // If first part, let output handler determine producer
            if( output_handler( &producer, &producer_data, output_handler_data ) != 0 )
            {
                server->logger_f( server->logger_d, 0, who,
                                  "Output handler failed." );
                ret = -1;
                server->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
                ipc_msg_server_signal_client( errnobuf, errnobuf_size, server );
                break;
            }
            if( producer == NULL ) {
                server->logger_f( server->logger_d, 0, who,
                                  "Output handler returned nullptr." );
                ret = -2;
                server->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
                ipc_msg_server_signal_client( errnobuf, errnobuf_size, server );
                break;
            }
        }

        // --- check that state is as expected (and not error) -----------------
        if( server->shmem_header_ptr->state != IPC_MSG_STATE_SERVER_TO_CLIENT ) {
            server->logger_f( server->logger_d, 0, who,
                              "Got state %d, expected state %d.",
                              server->shmem_header_ptr->state,
                              IPC_MSG_STATE_SERVER_TO_CLIENT );
            ret = -1;
            server->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
            ipc_msg_server_signal_client( errnobuf, errnobuf_size, server );
            break;
        }

        // --- produce message part --------------------------------------------
        int more = 0;
        size_t bytes = 0;
        if( producer( producer_data,
                      &more,
                      (char*)server->shmem_payload_ptr,
                      &bytes,
                      server->shmem_payload_size,
                      part ) != 0 )
        {
            server->logger_f( server->logger_d, 0, who, "Producer failed." );
            ret = -1;
            server->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
            ipc_msg_server_signal_client( errnobuf, errnobuf_size, server );
            break;
        }
        
        // --- signal the client that it might process this part ---------------
        server->shmem_header_ptr->part  = part;
        server->shmem_header_ptr->more  = more;
        server->shmem_header_ptr->bytes = bytes;
        if( (ret = ipc_msg_server_signal_client( errnobuf, errnobuf_size, server ) ) != 0 ) {
            break;
        }
        
        // --- if this was the last part, break out ----------------------------
        if( more == 0 ) {
            break;
        }

        // --- wait for buffer to be ready for the next part -------------------        
        if( (ret = ipc_msg_server_wait_client( errnobuf, errnobuf_size, server ) ) != 0 ) {
            break;
        }
    }
    return ret;
}





int
ipc_msg_server_notify( tinia_ipc_msg_server_t* server )
{
    static const char* who = "tinia.ipc.msg.server.mainloop.notify";
    char errnobuf[256];
    int rc, ret = 0;

    if( pthread_equal( pthread_self(), server->thread_id ) != 0  ) {
#ifdef DEBUG
        server->logger_f( server->logger_d, 2, who, "Invoked from mainloop thread." );
#endif
        // --- we're the mainloop thread -------------------------------------------

        // --- signal notification condition (linked to transaction lock) ------
        rc = pthread_cond_broadcast( &server->shmem_header_ptr->notification_event );
        if( rc != 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "pthread_cond_broadcast( notification_event ) failed: %s",
                              ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
            ret = -2;
        }

    }
    else {
#ifdef DEBUG
        server->logger_f( server->logger_d, 2, who, "Invoked from non-mainloop thread." );
#endif
        // --- we're not the mainloop thread -----------------------------------
        //
        // Preferably, we should lock the transaction lock, to avoid that the
        // notification appears right after a client has requested the status,
        // but before it has managed to start listening. I.e., clients may miss
        // longpolling updates and hang around until timeout.
        //
        // However, with the current implementation of exposed model, locking
        // the transaction lock may lead to the following deadlock:
        //
        // Non-mainloop job thread:
        // - updating a value
        // - triggering a notify via an exposed-model listener:
        //   - it has the exposedmodel lock
        //   - waits on the transaction lock (in order to signal condition)
        //
        // Apache mod_trell thread interacting with an ipc client:
        // - queries for updates
        //   - has the transaction lock
        //   - has sent the request to the ipc server
        //   - waits on a reply from the server
        //
        // IPCController mainloop thread running an ipc server:
        // - got a query for updates from client:
        //   - waits on exposed model lock (in order to check for updates).
        //
        // The optimal fix for this is to make sure that this function is not
        // invoked while holding the exposed model lock.
#if 1
        // --- signal notification condition (linked to transaction lock) --
        rc = pthread_cond_broadcast( &server->shmem_header_ptr->notification_event );
        if( rc != 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "pthread_cond_broadcast( notification_event ) failed: %s",
                              ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
            ret = -2;
        }
#else
        // --- lock transaction lock -------------------------------------------
        struct timespec timeout;
        if( clock_gettime( CLOCK_REALTIME, &timeout ) != 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "clock_gettime( CLOCK_REALTIME ): %s",
                              ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
            ret = -2;
        }
        else {
            timeout.tv_sec += 1;
            rc = pthread_mutex_timedlock( &server->shmem_header_ptr->transaction_lock,
                                          &timeout );
            if( rc != 0 ) {
                server->logger_f( server->logger_d, 0, who,
                                  "pthread_mutex_timedlock( transaction_lock ) failed: %s",
                                  ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                ret = -2;
            }
            else {
                // --- signal notification condition (linked to transaction lock) --
                rc = pthread_cond_broadcast( &server->shmem_header_ptr->notification_event );
                if( rc != 0 ) {
                    server->logger_f( server->logger_d, 0, who,
                                      "pthread_cond_broadcast( notification_event ) failed: %s",
                                      ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                    ret = -2;
                }
                
                // --- unlock transaction lock -------------------------------------
                rc = pthread_mutex_unlock( &server->shmem_header_ptr->transaction_lock );
                if( rc != 0 ) {
                    server->logger_f( server->logger_d, 0, who,
                                      "pthread_mutex_unlock( transaction_lock ) failed: %s",
                                      ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                    ret = -2;
                }
            }
        }
#endif
    }
    
    return ret;
}

int
ipc_msg_server_mainloop_break( tinia_ipc_msg_server_t* server )
{
    static const char* who = "tinia.ipc.msg.server.mainloop.break";
    char errnobuf[256];
    int rc, ret = 0;
    
    // --- we're the mainloop thread -------------------------------------------
    if( pthread_equal( pthread_self(), server->thread_id ) != 0  ) {

        server->logger_f( server->logger_d, 2, who, "Invoked from mainloop thread." );
        server->shmem_header_ptr->mainloop_running = 0;

    }
    // --- we're not the mainloop thread ---------------------------------------
    // The server is likely to be waiting on the server_event condition, so we
    // must signal that to make the server notice that we have changed the
    // mainloop_running flag. Also, we don't want to intervene with a
    // transaction if one is in progress, so we must take the transaction lock
    // as well.
    else {
        server->logger_f( server->logger_d, 2, who, "Invoked from non-mainloop thread." );

        struct timespec timeout;
        
        if( clock_gettime( CLOCK_REALTIME, &timeout ) != 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "clock_gettime( CLOCK_REALTIME ): %s",
                              ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
            ret = -2;
        }
        else {
            timeout.tv_sec += 1;
            
            // --- take transaction lock ---------------------------------------
            rc = pthread_mutex_timedlock( &server->shmem_header_ptr->transaction_lock,
                                          &timeout );
            if( rc != 0 ) {
                server->logger_f( server->logger_d, 0, who,
                                  "pthread_mutex_timedlock( transaction_lock ) failed: %s",
                                  ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                ret = -2;
            }
            else {
                // --- take operation lock -------------------------------------
                rc = pthread_mutex_timedlock( &server->shmem_header_ptr->operation_lock,
                                              &timeout );
                if( rc != 0 ) {
                    server->logger_f( server->logger_d, 0, who,
                                      "pthread_mutex_lock( operation_lock ) failed: %s",
                                      ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                    ret = -2;
                }
                else {
                    // --- set flags and signal server event -------------------
                    server->shmem_header_ptr->mainloop_running = 0;
                    server->shmem_header_ptr->server_event_predicate = 1;
                    rc = pthread_cond_signal( &server->shmem_header_ptr->server_event );
                    if( rc != 0 ) {
                        server->logger_f( server->logger_d, 0, who,
                                          "pthread_cond_signal( server_event ) failed: %s",
                                          ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                        ret = -2;
                    }
                    // --- release operation lock ------------------------------
                    rc = pthread_mutex_unlock( &server->shmem_header_ptr->operation_lock );
                    if( rc != 0 ) {
                        server->logger_f( server->logger_d, 0, who,
                                          "pthread_mutex_unlock( operation_lock ) failed: %s",
                                          ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                        ret = -2;
                    }
                }
                // --- release transaction lock --------------------------------
                rc = pthread_mutex_unlock( &server->shmem_header_ptr->transaction_lock );
                if( rc != 0 ) {
                    server->logger_f( server->logger_d, 0, who,
                                      "pthread_mutex_unlock( transaction_lock ) failed: %s",
                                      ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                    ret = -2;
                }
            }
        }
    }
   
    return ret;
}



void*
ipc_msg_server_signal_thread( void* data )
{
    static const char* who = "tinia.ipc.msg.signal.thread";
    char errnobuf[256];
    int rc;
    
    tinia_ipc_msg_server_t* server = (tinia_ipc_msg_server_t*)data;
    while(1) {

        // --- wait on signals (SIGTERM) ---------------------------------------
        int signal;
        sigset_t sigset;
        sigemptyset( &sigset );
        sigaddset( &sigset, SIGTERM );
        rc = sigwait( &sigset, &signal );    // cancellation point
        if( rc != 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "sigwait failed: %s",
                              ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
            break;
        }
        
        // --- disable cancelling of this thread -------------------------------
        // we don't want to be cancelled while we're holding any locks:
        int old_state;
        rc = pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, &old_state );
        if( rc != 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "pthread_setcancelstate(PTHREAD_CANCEL_DISABLE) failed: %s",
                              ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
        }
        else {
            // --- try to break mainloop ---------------------------------------
            if( ipc_msg_server_mainloop_break( server ) != 0 ) {
                server->logger_f( server->logger_d, 0, who,
                                  "Failed to break mainloop" );
            }
            // --- re-enable this thread as cancellable ------------------------
            rc = pthread_setcancelstate( old_state, NULL );
            if( rc != 0 ) {
                server->logger_f( server->logger_d, 0, who,
                                  "pthread_setcancelstate(old_state) failed: %s",
                                  ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
            }
        }
    }
    return NULL;
}


int
ipc_msg_server_mainloop_iteration( char* errnobuf,
                                   size_t errnobuf_size,
                                   struct timespec* periodic_timeout,
                                   tinia_ipc_msg_server_t* server,
                                   tinia_ipc_msg_periodic_func_t periodic, void* periodic_data,
                                   tinia_ipc_msg_input_handler_func_t input_handler, void* input_handler_data,
                                   tinia_ipc_msg_output_handler_func_t output_handler, void* output_handler_data )
{
    static const char* who = "tinia.ipc.msg.server.mainloop.iteration";
    int rc;

#ifdef TRACE
    server->logger_f( server->logger_d, 2, who, "Starting new listen iteration." );
#endif
    
    // --- signal client that might be waiting on us ---------------------------
    server->shmem_header_ptr->state = IPC_MSG_STATE_READY;
    rc = pthread_cond_signal( &server->shmem_header_ptr->client_event );
    if( rc != 0 ) {
        server->logger_f( server->logger_d, 0, who,
                          "pthread_cond_signal( client_event ): %s",
                          ipc_msg_strerror_wrap(rc, errnobuf, errnobuf_size ) );
        return -2;
    }

    // --- wait for a server event -----------------------------------------
    server->shmem_header_ptr->server_event_predicate = 0;
    do {
        rc = pthread_cond_timedwait( &server->shmem_header_ptr->server_event,
                                     &server->shmem_header_ptr->operation_lock,
                                     periodic_timeout );
        // --- timeout, invoke periodic function & update timeout ----------
        if( rc == ETIMEDOUT ) {
            rc = periodic( periodic_data );
            if( rc != 0 ) {
                server->logger_f( server->logger_d, 0, who,
                                  "periodic func returned: %d", rc );
                return -2;
            }
            rc = clock_gettime( CLOCK_REALTIME, periodic_timeout );
            if( rc != 0 ) {
                server->logger_f( server->logger_d, 0, who,
                                  "clock_gettime failed: %s",
                                  ipc_msg_strerror_wrap(rc, errnobuf, errnobuf_size ) );
                return -2;
            }
            periodic_timeout->tv_sec += 3;
        }
        // --- error, bail out ---------------------------------------------
        else if( rc != 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "pthread_cond_timedwait( server_event) failed: %s",
                              ipc_msg_strerror_wrap(rc, errnobuf, errnobuf_size ) );
            return -2;
        }
    }
    while( server->shmem_header_ptr->server_event_predicate == 0 );

    if( server->shmem_header_ptr->mainloop_running == 0 ) {
        return 0;
    }

    // --- handle incoming message ---------------------------------------------
    // --- check that state is as expected (and not error) -----------------
    if( server->shmem_header_ptr->state != IPC_MSG_STATE_CLIENT_TO_SERVER ) {
        server->logger_f( server->logger_d, 0, who,
                          "Got state %d, expected state %d.",
                          server->shmem_header_ptr->state,
                          IPC_MSG_STATE_CLIENT_TO_SERVER );
        server->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
        //ipc_msg_server_signal_client( errnobuf, errnobuf_size, server );
        return -1;
    }
    
    rc = ipc_msg_server_recv( errnobuf,
                              errnobuf_size,
                              server,
                              input_handler,
                              input_handler_data );
    if( rc != 0) {
        server->logger_f( server->logger_d, 1, who, "Receive failed." );
        return rc;
    }
    
    // --- send outgoing message -----------------------------------------------
    server->shmem_header_ptr->state = IPC_MSG_STATE_SERVER_TO_CLIENT;
    rc = ipc_msg_server_send( errnobuf,
                                     errnobuf_size,
                                     server,
                                     output_handler,
                                     output_handler_data );
    if( rc != 0) {
        server->logger_f( server->logger_d, 1, who, "Send failed." );
        return rc;
    }
    
    // --- wait for client to finish -------------------------------------------
    struct timespec timeout;
    clock_gettime( CLOCK_REALTIME, &timeout );
    if( rc != 0 ) {
        server->logger_f( server->logger_d, 0, who,
                          "clock_gettime failed: %s",
                          ipc_msg_strerror_wrap(rc, errnobuf, errnobuf_size ) );
        return -2;
    }
    timeout.tv_sec += 1;

    do {
        rc = pthread_cond_timedwait( &server->shmem_header_ptr->server_event,
                                     &server->shmem_header_ptr->operation_lock,
                                     &timeout );
        if( rc == ETIMEDOUT ) {
            server->logger_f( server->logger_d, 0, who, "Timeout." );
            return -1;
        }
        else if( rc != 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "pthread_cond_timedwait( server_event) failed: %s",
                              ipc_msg_strerror_wrap(rc, errnobuf, errnobuf_size ) );
            return -2;
        }
    }
    while( server->shmem_header_ptr->state == IPC_MSG_STATE_SERVER_TO_CLIENT );

    return 0;
}

int
ipc_msg_server_mainloop( tinia_ipc_msg_server_t* server,
                         tinia_ipc_msg_periodic_func_t periodic, void* periodic_data,
                         tinia_ipc_msg_input_handler_func_t input_handler, void* input_handler_data,
                         tinia_ipc_msg_output_handler_func_t output_handler, void* output_handler_data )
{
    static const char* who = "tinia.ipc.msg.server.mainloop";
    char errnobuf[256];
    int ret = 0, rc;
    
    // --- make sure that we are the right thread ------------------------------
    if( pthread_equal( pthread_self(), server->thread_id ) == 0 ) {
        server->logger_f( server->logger_d, 0, who,
                          "Mainloop and init invoked in different threads." );
        return -2;
    }

    struct timespec timeout;
    if( clock_gettime( CLOCK_REALTIME, &timeout ) != 0 ) {
        server->logger_f( server->logger_d, 0, who,
                          "clock_gettime( CLOCK_REALTIME ): %s",
                          ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
        ret = -2;
    }
    else {
        timeout.tv_sec += 1;
    
        // --- don't let any clients commence a transaction before we are ready
        rc = pthread_mutex_timedlock( &server->shmem_header_ptr->operation_lock,
                                      &timeout );
        if( rc != 0 ) {
            server->logger_f( server->logger_d, 0, who,
                              "Failed to lock operation lock: %s",
                              ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
            ret = -2;
        }
        else {
            // --- set up signal checker thread --------------------------------
            
            // signal checker thread will set this variable to zero when it
            // receives a SIGTERM.
            server->shmem_header_ptr->mainloop_running = 1;
            
            pthread_t signal_checker;
            rc = pthread_create( &signal_checker,
                                 NULL,
                                 ipc_msg_server_signal_thread,
                                 server );
            if( rc != 0 ) {
                server->logger_f( server->logger_d, 0, who,
                                  "Failed to create signal checker thread: %s",
                                  ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                ret = -2;
            }
            else {
                // --- set up initial periodic timeout -----------------------------
                struct timespec periodic_timeout;
                rc = clock_gettime( CLOCK_REALTIME, &periodic_timeout );
                if( rc != 0 ) {
                    server->logger_f( server->logger_d, 0, who,
                                      "clock_gettime failed: %s",
                                      ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                    ret = -2;
                }
                
                // --- run mainlooop iterations ------------------------------------
                while( (ret > -2 ) && (server->shmem_header_ptr->mainloop_running != 0) ) {
                    ret = ipc_msg_server_mainloop_iteration( errnobuf, sizeof(errnobuf), &periodic_timeout,
                                                             server, periodic, periodic_data,
                                                             input_handler, input_handler_data,
                                                             output_handler, output_handler_data );
                }
                server->logger_f( server->logger_d, 2, who,
                                  "Breaking the main loop." );
                server->shmem_header_ptr->state = IPC_MSG_STATE_DONE;
                
                // --- terminate signal checker thread -----------------------------
                void* retval;
                rc = pthread_cancel( signal_checker );
                if( rc != 0 ) {
                    server->logger_f( server->logger_d, 1, who,
                                      "Failed to cancel signal checker thread: %s",
                                      ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                }
                rc = pthread_join( signal_checker, &retval );
                if( rc != 0 ) {
                    server->logger_f( server->logger_d, 1, who,
                                      "Failed to join signal checker thread: %s",
                                      ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                }
            }
            // --- unlock operation lock ---------------------------------------
            rc = pthread_mutex_unlock( &server->shmem_header_ptr->operation_lock );
            if( rc != 0 ) {
                server->logger_f( server->logger_d, 0, who,
                                  "Failed to unlock operation lock: %s",
                                  ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                ret = -1;
            } 
        }
    }
    
    return ret;
}
