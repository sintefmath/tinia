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
#include <fcntl.h>      // O_* constants
#include <errno.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>       // memcpy
#include "tinia/ipc/messenger.h"

#define PREFIX "mod_trell: @messenger: "

static const char* package = "ipc.messenger";

const char*
messenger_strerror( messenger_status_t error )
{
    switch( error ) {
    case MESSENGER_OK:
        return "Ok";
        break;
    case MESSENGER_ERROR:
        return "Unspecified error";
        break;
    case MESSENGER_NULL:
        return "Received null pointer";
        break;
    case MESSENGER_SHMEM_LOCKED:
        return "Lock exists for shared memory range";
        break;
    case MESSENGER_INVARIANT_BROKEN:
        return "Invariant broken";
        break;
    case MESSENGER_INVALID_MBOX:
        return "Invalid message box";
        break;
    case MESSENGER_OPEN_FAILED:
        return "Open failed";
        break;
    case MESSENGER_TIMEOUT:
        return "Timeout";
        break;
    case MESSENGER_INTERRUPTED:
        return "Interrupted by a signal handler";
        break;
    }
    return NULL;
}





static int
messenger_sem_open( sem_t** sem, const char* format, const char* jobname )
{
    char buffer[ 256 ];
    if( snprintf( buffer, sizeof(buffer), format, jobname ) >= sizeof(buffer) ) {
        fprintf( stderr, PREFIX "formatting buffer too small\n" );
        *sem = SEM_FAILED;
        return -1;
    }
    *sem = sem_open( buffer, 0 );
    if( *sem == SEM_FAILED ) {
        fprintf( stderr, PREFIX "failed to open %s\n", buffer );
        return -1;
    }
    return 0;
}

static messenger_status_t
messenger_shm_open( void** memory, size_t* memory_size, const char* format, const char* jobname )
{
    messenger_status_t rv = MESSENGER_ERROR;

    char buffer[ 256 ];
    if( snprintf( buffer, sizeof(buffer), format, jobname ) >= sizeof(buffer) ) {
        fprintf( stderr, PREFIX "formatting buffer too small\n" );
        *memory = MAP_FAILED;
        *memory_size = 0;
        rv = MESSENGER_ERROR;
    }
    else {

        // Open shared memory file descriptor
        int fd = shm_open( buffer, O_RDWR, 0666 );
        if( fd < 0 ) {
            perror( PREFIX "failed to open shared memory" );
            *memory = MAP_FAILED;
            *memory_size = 0;
            rv = MESSENGER_INVALID_MBOX;
        }
        else {

            // Determine size of memory region
            struct stat fstat_buf;
            if( fstat( fd, &fstat_buf ) != 0 ) {
                perror( PREFIX "failed to stat shared memory to determine size" );
                close( fd );
                *memory = MAP_FAILED;
                *memory_size = 0;
                rv = MESSENGER_ERROR;
            }
            else {
                *memory_size = fstat_buf.st_size;

                // Map memory region
                *memory = mmap( NULL,
                                *memory_size,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED,
                                fd,
                                0 );
                if( *memory == MAP_FAILED ) {
                    perror( PREFIX "mmap of shared memory failed" );
                    rv = MESSENGER_ERROR;
                }
                else {
                    // should be open now.
                    rv = MESSENGER_OK;
                }
            }
            close( fd );
        }
    }
    return rv;
}

messenger_status_t
messenger_endpoint_create( messenger_server_t* e, const char* id )
{
    e->m_shmem_ptr = MAP_FAILED;
    e->m_shmem_size = 0;
    e->m_shmem_name = NULL;
    e->m_sem_lock = SEM_FAILED;
    e->m_sem_lock_name = NULL;
    e->m_sem_query = SEM_FAILED;
    e->m_sem_query_name = NULL;
    e->m_sem_reply = SEM_FAILED;
    e->m_sem_reply_name = NULL;
    e->m_notify = 0;
    e->m_sem_notify = SEM_FAILED;
    e->m_sem_notify_name = NULL;


    return MESSENGER_OK;
}

messenger_status_t
messenger_endpoint_destroy( messenger_server_t* e )
{
    return MESSENGER_OK;
}


messenger_status_t
messenger_init( struct messenger* m,
                const char* jobname,
                void   (*logger_func)( void *data, int level, const char* who, const char* msg, ... ),
                void*    logger_data )
{
    m->m_has_lock = 0;
    m->m_shmem_ptr = MAP_FAILED;
    m->m_shmem_size = 0u;
    m->m_lock = SEM_FAILED;
    m->m_query = SEM_FAILED;
    m->m_reply = SEM_FAILED;
    m->m_notify = SEM_FAILED;
    m->m_logger_func = logger_func;
    m->m_logger_data = logger_data;

    if( jobname == NULL ) {
        return MESSENGER_INVALID_MBOX;
    }
    size_t l=0;
    const char* p;
    for( p = jobname; *p != '\0'; p++ ) {
        if( l++ > 127 ) {
            return MESSENGER_INVALID_MBOX;
        }
        if( !(isalnum(*p) || *p=='_' ) ) {
            fprintf( stderr, "%s: illegal char %d: %s", __func__, *p, jobname );
            return MESSENGER_INVALID_MBOX;
        }
    }
    if( ( messenger_shm_open( &m->m_shmem_ptr, &m->m_shmem_size, "/%s_shmem", jobname ) != 0 ) ||
        ( messenger_sem_open( &m->m_lock, "/%s_lock", jobname ) != 0 ) ||
        ( messenger_sem_open( &m->m_query, "/%s_query", jobname ) != 0 ) ||
        ( messenger_sem_open( &m->m_reply, "%s_reply", jobname ) != 0 ) ||
        ( messenger_sem_open( &m->m_notify, "%s_notify", jobname ) != 0 ) )
    {
        messenger_free( m );
        return MESSENGER_INVALID_MBOX;
    }
    return MESSENGER_OK;
}



// -----------------------------------------------------------------------------
// this should end up in messenger.c when it is working.
messenger_status_t
messenger_do_roundtrip_cb( messenger_producer_t query, void* query_data,
                           messenger_consumer_t reply, void* reply_data,
                           messenger_logger_t log, void* log_data,
                           const char* message_box_id,
                           int wait /* in seconds. */ )
{
    messenger_status_t status = MESSENGER_OK;
    messenger_t msgr;
    messenger_status_t mrv;

    // --- open connection to messenger ----------------------------------------
    mrv = messenger_init( &msgr, message_box_id,
                          log,
                          log_data );
    if( mrv != MESSENGER_OK ) {
        msgr.m_logger_func( msgr.m_logger_data, 0, package, "messenger_init: %s", messenger_strerror( mrv ) );
        status = MESSENGER_ERROR;
    }
    else {
        // --- create timeout for request --------------------------------------
        struct timespec timeout;
        clock_gettime( CLOCK_REALTIME, &timeout );
        timeout.tv_sec += wait + 1;

        // --- begin query loop (which may be broken after 1 iteration) --------
        int do_longpoll;
        do {

            do_longpoll = 0;    // will be set to 1 if we will longpoll
        
            // --- try to lock messenger  --------------------------------------
            if( sem_timedwait( msgr.m_lock, &timeout ) != 0 ) {
                switch( errno ) {
                case EINTR:
                case ETIMEDOUT: status = MESSENGER_TIMEOUT; break;
                default:        status = MESSENGER_ERROR; break;
                }
                log( log_data, 0, package, "sem_timedwait(lock): %s", strerror(errno) );
            }
            else {
                if( sem_trywait( msgr.m_reply ) == 0 ) {
                    msgr.m_logger_func( msgr.m_logger_data, 1, package, "Pulled an old up from the reply semaphore.\n" );
                }

                volatile messenger_header_t* header = msgr.m_shmem_ptr;
                
                // --- create query in shmem (read request contents) -----------
                size_t bytes_written = 0;
                int q = query(  query_data,
                               &bytes_written,
                                msgr.m_shmem_ptr + sizeof(messenger_header_t),
                                msgr.m_shmem_size - sizeof(messenger_header_t) );
                header->m_size = bytes_written;
                header->m_more = 0;
                header->m_error = 0;
                if( q < 0 ) {
                    msgr.m_logger_func( msgr.m_logger_data, 0, package, "query callback failed." );
                    status = MESSENGER_ERROR;
                }
                else if( q > 0 ) {
                    msgr.m_logger_func( msgr.m_logger_data, 0, package, "Message size exceeds shared memory size, currently unsupported." );
                    status = MESSENGER_ERROR;
                }
                else {
                    
                    // --- sync memory TO other process ------------------------
                    // sync memory FIXME: size
                    if( msync( msgr.m_shmem_ptr,
                               header->m_size + sizeof(messenger_header_t),
                               MS_SYNC | MS_INVALIDATE ) != 0 )
                    {
                        status = MESSENGER_ERROR; 
                        log( log_data, 0, package, "msync(send): %s", strerror(errno) );
                    }
                    // --- notify other porcess that we have a message ---------
                    else if( sem_post( msgr.m_query ) != 0 ) {
                        switch( errno ) {
                        case EINVAL:    status = MESSENGER_INVALID_MBOX;     break;
                        case EOVERFLOW: status = MESSENGER_INVARIANT_BROKEN; break;
                        default:        status = MESSENGER_ERROR;            break;
                        }
                        log( log_data, 0, package, "sem_post(query): %s", strerror(errno) );
                    }
                    else {
                        struct timespec timeout;
                        clock_gettime( CLOCK_REALTIME, &timeout );
                        timeout.tv_sec += 1;
                        
                        // --- wait for other process to finish processing -----
                        int redo;
                        do {
                            redo = 0;
                            if( sem_timedwait( msgr.m_reply, &timeout ) != 0 ) {
                                if( errno == EINTR ) {
                                    redo = 1;   // we might get interrupted, just hang in there...
                                }
                                else {
                                    log( log_data, 0, package, "sem_timedwait(reply): %s", strerror(errno) );
                                    status = MESSENGER_ERROR;
                                }
                            }
                        } while( redo );
                       
                        if( status == MESSENGER_OK ) {
                            if( msync( msgr.m_shmem_ptr,
                                       msgr.m_shmem_size,
                                       MS_SYNC ) != 0 )
                            {
                                log( log_data, 0, package, "msync(recv): %s", strerror(errno) );
                                status = MESSENGER_ERROR;
                            }
                        }

                        if( status == MESSENGER_OK ) {
                            // --- process reply (write request contents) ----------
                            int r = reply( reply_data,
                                           msgr.m_shmem_ptr + sizeof(messenger_header_t),
                                           0,
                                           msgr.m_shmem_size - sizeof(messenger_header_t), 0 );
                            if( r < 0 ) {
                                msgr.m_logger_func( msgr.m_logger_data, 0, package, "reply callback failed." );
                                status = MESSENGER_ERROR;
                            }
                            else if( r == 0 ) {
                                status = MESSENGER_OK;
                            }
                            else {
                                status = MESSENGER_TIMEOUT;
                                do_longpoll = wait > 0 ? 1 : 0;
                            }
                        }
                    }
                }

                // --- free messenger and let others use it --------------------
                if( sem_post( msgr.m_lock ) != 0 ) {
                    log( log_data, 0, package, "sem_post(lock): %s", strerror(errno) );
                    status = MESSENGER_ERROR;
                }
                // -------------------------------------------------------------
                // NOTE:
                // We can miss an update here, should do unlock-lock in atomic
                // operations, but it seems impossible with the currently used
                // semaphores. semop might be a solution.
                // -------------------------------------------------------------

                // --- should we wait (i.e. longpoll?) -------------------------
                if( !do_longpoll ) {
                    break;
                }
                
                // --- check if we already have waited more than timeout -------
                struct timespec current;
                clock_gettime( CLOCK_REALTIME, &current );
                if( (current.tv_sec > timeout.tv_sec )
                        || ( (current.tv_sec == timeout.tv_sec )
                             && (current.tv_nsec > timeout.tv_nsec) ) )
                {
                    break;
                }
                
                // --- try to wait for a notification --------------------------
                if( sem_timedwait( msgr.m_notify, &timeout ) < 0 ) {
                    switch ( errno ) {
                    case EINTR: // just interrupted, just check and redo
                        msgr.m_logger_func( msgr.m_logger_data, 1, package, "sem_timedwait: woken by an interrupt." );
                        break;
                    case EINVAL: // invalid semaphore, give up
                        msgr.m_logger_func( msgr.m_logger_data, 0, package, "sem_timedwait: Invalid sempahore." );
                        do_longpoll = 0;
                        break;
                    case ETIMEDOUT:
                        status = MESSENGER_TIMEOUT;
                        do_longpoll = 0;
                        break;
                    default:
                        msgr.m_logger_func( msgr.m_logger_data, 0, package, "sem_timedwait: Unexpected error %d.", errno );
                        status = MESSENGER_ERROR;
                        do_longpoll = 0;
                        break;
                    }
                }
                else {
                    // we do have an update.
                    msgr.m_logger_func( msgr.m_logger_data, 1, package, "sem_timedwait: got notified." );
                }
            }
        }
        while( do_longpoll );
            
        // close connection to messenger        
        mrv = messenger_free( &msgr );
        if( mrv != MESSENGER_OK ) {
            msgr.m_logger_func( msgr.m_logger_data, 0, package, "messenger_free: %s", messenger_strerror( mrv ) );
            status = MESSENGER_ERROR;
        }
    }
    return status;
}

// --- do roundtrip no-callback convenience func -------------------------------
typedef struct messenger_do_roundtrip_data {
    const char*         m_query;            // Query buffer.
    size_t              m_query_sent;       // Number of bytes read from query buffer.
    size_t              m_query_size;       // Size of query buffer.
    char*               m_reply;            // Reply buffer.
    size_t              m_reply_received;   // Number of bytes written to reply buffer.
    size_t              m_reply_size;       // Size of reply buffer.
    messenger_status_t  m_status;
} messenger_do_roundtrip_data_t;

static
int
messenger_do_roundtrip_producer( void*           data,
                                 size_t*         bytes_written,
                                 unsigned char*  buffer,
                                 size_t          buffer_size )
{
    int retval = 0;
    messenger_do_roundtrip_data_t * pd = (messenger_do_roundtrip_data_t*)data;
    size_t size = pd->m_query_size-pd->m_query_sent;
    if( buffer_size < size ) {
        size = buffer_size;
        retval = 1; // we need more invocations
    }
    memcpy( buffer, pd->m_query + pd->m_query_sent, size );
    pd->m_query_sent += size;
    *bytes_written = size;
    return retval;
}

static
int
messenger_do_roundtrip_consumer(  void* data,
                                  unsigned char* buffer,
                                  size_t offset,
                                  size_t bytes,
                                  int more )
{
    int retval = 0;
    messenger_do_roundtrip_data_t * pd = (messenger_do_roundtrip_data_t*)data;
    size_t size = pd->m_reply_size-pd->m_reply_received;
    if( bytes <= size ) {
        size = bytes;
    }
    else {
        // reply buffer is too small! Currently bytes is size of shared mem
        //pd->m_status = MESSENGER_INSUFFICIENT_MEMORY;
        //retval = -1;
    }
    memcpy( pd->m_reply + pd->m_reply_received, buffer, size );
    pd->m_reply_received += size;
    return 0;
}

messenger_status_t
messenger_do_roundtrip( const void *query, size_t query_size,
                        void *reply, size_t* reply_written, size_t reply_size,
                        messenger_logger_t log, void* log_data,
                        const char* message_box_id,
                        int wait )
{
    messenger_do_roundtrip_data_t data = {
        query, 0, query_size,
        reply, 0, reply_size,
        MESSENGER_OK
    };
    messenger_status_t rv = messenger_do_roundtrip_cb( messenger_do_roundtrip_producer, &data,
                                                       messenger_do_roundtrip_consumer, &data,
                                                       log, log_data,
                                                       message_box_id,
                                                       wait );
    *reply_written = data.m_reply_received;
    if( data.m_status == MESSENGER_INSUFFICIENT_MEMORY ) {
        log( log_data, 1, package, "do_roundtrip: Insufficient size of query buffer." );
        return MESSENGER_INSUFFICIENT_MEMORY;
    }
    else {
        return rv;
    }
}



messenger_status_t
messenger_free( struct messenger* m )
{
    if( m == NULL ) {
        return MESSENGER_NULL;
    }
    messenger_status_t rv = MESSENGER_OK;
    if( m->m_has_lock != 0 ) {
        rv = MESSENGER_INVARIANT_BROKEN;
    }
    if( m->m_shmem_ptr != MAP_FAILED ) {
        munmap( m->m_shmem_ptr, m->m_shmem_size );
        m->m_shmem_ptr = MAP_FAILED;
    }
    if( m->m_lock != SEM_FAILED ) {
        sem_close( m->m_lock );
        m->m_lock = SEM_FAILED;
    }
    if( m->m_query != SEM_FAILED ) {
        sem_close( m->m_query );
        m->m_query = SEM_FAILED;
    }
    if( m->m_reply != SEM_FAILED ) {
        sem_close( m->m_reply );
        m->m_reply = SEM_FAILED;
    }
    return rv;
}


messenger_status_t
messenger_wait_for_notification( struct messenger* m, int wait_seconds )
{

    struct timespec timeout;
    clock_gettime( CLOCK_REALTIME, &timeout );
    timeout.tv_sec += wait_seconds;

    while(1) {
        if( sem_timedwait( m->m_notify, &timeout ) == 0 ) {
            // got notification
            return MESSENGER_OK;
        }
        else {
            switch( errno ) {
            case EINTR:
                // Interrupted, just continue sleeping.
                break;
            case EINVAL:
                // Invalid sempahore
                return MESSENGER_INVALID_MBOX;
                break;
            case ETIMEDOUT:
                // Timed out
                return MESSENGER_TIMEOUT;
                break;
            default:
                // Shouldn't happen
                return MESSENGER_INVARIANT_BROKEN;
                break;
            }
       }
    }
    // Never reached
    return MESSENGER_ERROR;
}
#if 0


messenger_status_t
messenger_lock( struct messenger* m )
{
    if( m->m_has_lock != 0 ) {
        return MESSENGER_INVARIANT_BROKEN;
    }
    if( m->m_shmem_ptr == MAP_FAILED ) {
        return MESSENGER_INVALID_MBOX;
    }

    struct timespec timeout;
    clock_gettime( CLOCK_REALTIME, &timeout );
    timeout.tv_sec += 5;
    if( sem_timedwait( m->m_lock, &timeout ) != 0 ) {
        return MESSENGER_TIMEOUT;
    }
    // Got lock
    m->m_has_lock = 1;
    return MESSENGER_OK;
}


messenger_status_t
messenger_post( struct messenger*m, size_t size )
{
    if( sem_trywait( m->m_reply ) == 0 ) {
        fprintf( stderr, PREFIX "Pulled an old up from the reply semaphore.\n" );
    }

    if( msync( m->m_shmem_ptr, size, MS_SYNC | MS_INVALIDATE ) != 0 ) {
        switch( errno ) {
        case EBUSY:
            return MESSENGER_SHMEM_LOCKED;
            break;
        case EINVAL:
        case ENOMEM:
            return MESSENGER_ERROR;
            break;
        }
    }

    if( sem_post( m->m_query ) != 0 ) {
        switch( errno ) {
        case EINVAL:
            return MESSENGER_INVALID_MBOX;
            break;
        case EOVERFLOW:
            return MESSENGER_INVARIANT_BROKEN;
            break;
        default:
            return MESSENGER_ERROR;
            break;
        }
    }

    struct timespec timeout;
    clock_gettime( CLOCK_REALTIME, &timeout );
    timeout.tv_sec += 5;

    if( sem_timedwait( m->m_reply, &timeout ) != 0 ) {
        switch( errno ) {
        case EINTR:
            return MESSENGER_INTERRUPTED;
            break;
        case EINVAL:
            return MESSENGER_INVALID_MBOX;
            break;
        case ETIMEDOUT:
            return MESSENGER_TIMEOUT;
            break;
        default:
            return MESSENGER_ERROR;
            break;
        }
    }

    if( msync( m->m_shmem_ptr, m->m_shmem_size, MS_SYNC ) != 0 ) {
        switch( errno ) {
        case EBUSY:
        case EINVAL:
        case ENOMEM:
            return MESSENGER_ERROR;
            break;
        }
    }
    return MESSENGER_OK;
}
messenger_status_t
messenger_unlock( struct messenger* m )
{
    if( m->m_has_lock == 0 ) {
        return MESSENGER_INVARIANT_BROKEN;
    }
    if( sem_post( m->m_lock ) != 0 ) {
        return MESSENGER_ERROR;
    }
    m->m_has_lock = 0;
    return MESSENGER_OK;
}

#endif
