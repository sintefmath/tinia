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
#include "tinia/trell/messenger.h"


#define PREFIX "mod_trell: @messenger: "


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
messenger_init( struct messenger* m, const char* jobname )
{
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
    m->m_has_lock = 0;
    m->m_shmem_ptr = MAP_FAILED;
    m->m_shmem_size = 0u;
    m->m_lock = SEM_FAILED;
    m->m_query = SEM_FAILED;
    m->m_reply = SEM_FAILED;
    m->m_notify = SEM_FAILED;
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

