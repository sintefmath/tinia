#include <fcntl.h>           /* For O_* constants */
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include "tinia/ipc/messenger.h"

#define LOG_WRAP( s, level, where, ... ) \
do { \
    if( s->m_log_func != NULL ) { \
        s->m_log_func( s->m_log_data, level, where, __VA_ARGS__ ); \
    } \
} while( 0 )
#define LOG_ERROR( s, where, ... ) LOG_WRAP( s, 0, where, __VA_ARGS__ )
#define LOG_WARN( s, where, ... )  LOG_WRAP( s, 1, where, __VA_ARGS__ )
#define LOG_INFO( s, where, ... )  LOG_WRAP( s, 2, where, __VA_ARGS__ )

static
messenger_status_t
messenger_semaphore_delete( messenger_server_t*  s,
                            sem_t**              semaphore,
                            const char*          name )
{
    static const char* where = "ipc.messenger.semaphore.delete";
    messenger_status_t rv = MESSENGER_OK;

    // --- close semaphore -----------------------------------------------------
    if( *semaphore != SEM_FAILED ) {
        if( sem_close( *semaphore ) != 0 ) {
            LOG_ERROR( s, where, "Failed to close semaphore '%s': %s", name, strerror(errno));
            rv = MESSENGER_ERROR;
        }
    }
    
    // --- unlink semaphore ----------------------------------------------------
    if( sem_unlink( name ) != 0 ) {
        // It is not necessarily an error that this fails, as the semaphore
        // might already be unlinked, since this function is invoked a lot of
        // times in error handling.
        rv = MESSENGER_ERROR;
    }

    if( rv == MESSENGER_OK ) {
        LOG_INFO( s, where, "Deleted semaphore '%s'.", name );
    }
    return rv;
}

static
messenger_status_t
messenger_semaphore_create( messenger_server_t*  s,
                            sem_t**              semaphore,
                            const char*          name )
{
    static const char* where = "ipc.messenger.semaphore.create";

    // --- first, kill semaphore with this name if exists ----------------------
    if( sem_unlink( name ) == 0 ) {
        LOG_INFO( s, where, "Removed existing semaphore '%s'", name );
    }
    
    // --- create semaphore ----------------------------------------------------
    *semaphore = sem_open( name, O_CREAT|O_EXCL, 0600, 0 );
    if( *semaphore == SEM_FAILED ) {
        LOG_ERROR( s, where, "Failed to create semaphore '%s': %s", name, strerror(errno) );
        messenger_semaphore_delete( s, semaphore, name );
        return MESSENGER_ERROR;
    }
    LOG_INFO( s, where, "Created semaphore '%s'.", name );
    return MESSENGER_OK;
}

static
messenger_status_t
messenger_shmem_delete( messenger_server_t* s,
                        const char*   name )
{
    static const char* where = "ipc.messenger.shmem.delete";
    messenger_status_t rv = MESSENGER_OK;

    // --- remove mapping of shared mem into this process space ----------------    
    if( s->m_shmem_ptr != MAP_FAILED ) {
        if( munmap( s->m_shmem_ptr, s->m_shmem_size ) != 0 ) {
            LOG_ERROR( s, where, "Failed to unmap shared memory '%s': %s", name, strerror(errno) );
            rv = MESSENGER_ERROR;
        }
    }

    // --- delete shared memory instance ---------------------------------------
    if( shm_unlink( name ) != 0 ) {
        // It is not necessarily an error that this fails, as the shared memory
        // might already be unlinked, since this function is invoked a lot of
        // times in error handling.
        rv = MESSENGER_ERROR;
    }
    s->m_shmem_ptr = MAP_FAILED;
    s->m_shmem_size = 0;
    
    if( rv == MESSENGER_OK ) {
        LOG_INFO( s, where, "Deleted shared memory '%s'.", name );
    }
    return rv;
}

static
messenger_status_t
messenger_shmem_create( messenger_server_t* s,
                        const char*   name,
                        const size_t  minimum_size )
{
    static const char* where = "ipc.messenger.shmem.create";
    
    s->m_shmem_ptr = MAP_FAILED;
    s->m_shmem_size = 0;
    
    // --- first, kill memory with this name if exists -------------------------
    if( shm_unlink( name ) == 0 ) {
        LOG_INFO( s, where, "Removed existing shared memory '%s'", name );
    }

    // --- create and open -----------------------------------------------------
    // NOTE: mode 0600, only user can communicate. Change if necessary.
    int fd = shm_open( name, O_RDWR | O_CREAT | O_EXCL, 0600 );
    if( fd < 0 ) {
        LOG_ERROR( s, where, "Failed to create shared memory '%s': %s", name, strerror(errno) );
        messenger_shmem_delete( s, name );
        return MESSENGER_ERROR;
    }
    
    // --- set size and query actual size --------------------------------------
    if( ftruncate( fd, minimum_size ) != 0 ) {
        LOG_ERROR( s, where, "Failed to set shared memory '%s' size to %d: %s",
                   name, minimum_size, strerror(errno) );
        close( fd );
        messenger_shmem_delete( s, name );
        return MESSENGER_ERROR;
    }
    struct stat fstat_buf;
    if( fstat( fd, &fstat_buf ) != 0 ) {
        LOG_ERROR( s, where, "Failed to determine shared memory '%s' size: %s",
                   name, strerror(errno) );
        close( fd );
        messenger_shmem_delete( s, name );
        return MESSENGER_ERROR;
    }
    s->m_shmem_size = fstat_buf.st_size;

    // --- map memory into this process memory space ---------------------------
    s->m_shmem_ptr = mmap( NULL, s->m_shmem_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0 );
    if( s->m_shmem_ptr == MAP_FAILED ) {
        LOG_ERROR( s, where, "Failed to map shared memory '%s': %s", name, strerror(errno) );
        close( fd );
        messenger_shmem_delete( s, name );
        return MESSENGER_ERROR;
    }

    close( fd );
    LOG_INFO( s, where, "Created shared memory '%s' of size %d bytes.", name, s->m_shmem_size );
    return MESSENGER_OK;
}




messenger_status_t
messenger_server_create( messenger_server_t* s,
                         const char*         id,
                         messenger_logger_t  log_func,
                         void*               log_data )
{
    char buffer[ 256+32 ];
    const char* where = "ipc.messenger.server.create";
    
    s->m_log_func = log_func;
    s->m_log_data = log_data;
    s->m_shmem_ptr = MAP_FAILED;
    s->m_shmem_size = 0;
    s->m_sem_lock = SEM_FAILED;
    s->m_sem_query = SEM_FAILED;
    s->m_sem_reply = SEM_FAILED;
    s->m_notify = 0;
    s->m_sem_notify = SEM_FAILED;
    strncpy( s->m_name, id, sizeof(s->m_name ) );
    if( s->m_name[ sizeof(s->m_name)-1 ] != '\0' ) {
        LOG_ERROR( s, where, "id too long (%s).", id);
        return MESSENGER_ERROR;
    }
    
    // --- create shared memory ------------------------------------------------
    snprintf( buffer, sizeof(buffer), "/%s_shmem", s->m_name );    // name
    if( messenger_shmem_create( s, buffer, 100*1024*1024 ) != MESSENGER_OK ) {
        messenger_server_destroy( s );
        return MESSENGER_ERROR;
    }

    // --- create lock semaphore -----------------------------------------------
    snprintf( buffer, sizeof(buffer), "/%s_lock", s->m_name );    // name
    if( messenger_semaphore_create( s, &s->m_sem_lock, buffer ) != MESSENGER_OK ) {
        messenger_server_destroy( s );
        return MESSENGER_ERROR;
    }

    // --- create query semaphore ----------------------------------------------
    snprintf( buffer, sizeof(buffer), "/%s_query", s->m_name );    // name
    if( messenger_semaphore_create( s, &s->m_sem_query, buffer ) != MESSENGER_OK ) {
        messenger_server_destroy( s );
        return MESSENGER_ERROR;
    }

    // --- create reply semaphore ----------------------------------------------
    snprintf( buffer, sizeof(buffer), "/%s_reply", s->m_name );    // name
    if( messenger_semaphore_create( s, &s->m_sem_reply, buffer ) != MESSENGER_OK ) {
        messenger_server_destroy( s );
        return MESSENGER_ERROR;
    }

    // --- create notify semaphore ---------------------------------------------
    snprintf( buffer, sizeof(buffer), "/%s_notify", s->m_name );    // name
    if( messenger_semaphore_create( s, &s->m_sem_notify, buffer ) != MESSENGER_OK ) {
        messenger_server_destroy( s );
        return MESSENGER_ERROR;
    }
}

messenger_status_t
messenger_server_destroy( messenger_server_t* s )
{
    char buffer[ 256+32 ];
    messenger_status_t rv = MESSENGER_OK;

    // --- delete shared memory ------------------------------------------------
    snprintf( buffer, sizeof(buffer), "/%s_shmem", s->m_name );    // name
    if( messenger_shmem_delete( s, buffer ) != MESSENGER_OK ) {
        rv = MESSENGER_ERROR;
    }

    // --- delete lock semaphore -----------------------------------------------
    snprintf( buffer, sizeof(buffer), "/%s_lock", s->m_name );    // name
    if( messenger_semaphore_delete( s, &s->m_sem_lock, buffer ) != MESSENGER_OK ) {
        rv = MESSENGER_ERROR;
    }

    // --- delete query semaphore ----------------------------------------------
    snprintf( buffer, sizeof(buffer), "/%s_query", s->m_name );    // name
    if( messenger_semaphore_delete( s, &s->m_sem_query, buffer ) != MESSENGER_OK ) {
        rv = MESSENGER_ERROR;
    }

    // --- delete reply semaphore ----------------------------------------------
    snprintf( buffer, sizeof(buffer), "/%s_reply", s->m_name );    // name
    if( messenger_semaphore_delete( s, &s->m_sem_reply, buffer ) != MESSENGER_OK ) {
        rv = MESSENGER_ERROR;
    }

    // --- delete notify semaphore ---------------------------------------------
    snprintf( buffer, sizeof(buffer), "/%s_notify", s->m_name );    // name
    if( messenger_semaphore_delete( s, &s->m_sem_notify, buffer ) != MESSENGER_OK ) {
        rv = MESSENGER_ERROR;
    }

    return rv;
}


// when this one is nonzero, we have recieved a SIGTERM
static sig_atomic_t exit_flag = 0;

static
void
messenger_handler_SIGTERM( int foo) {
    exit_flag = 1;
}

static
void
messenger_handler_SIGALRM( int foo ) {
    // do nothing.
}


messenger_status_t
messenger_server_notify( messenger_server_t* s ) 
{
    // set flag that there is something to notify
    s->m_notify = 1;
    // break if mainloop is in waitstate
    kill( getpid(), SIGALRM );
}

static
messenger_status_t
messenger_server_notified( messenger_server_t* s )
{
    messenger_status_t rv = MESSENGER_OK;
    static const char* where = "ipc.messenger.server.notified";
    struct timespec timeout;
    clock_gettime( CLOCK_REALTIME, &timeout );
    timeout.tv_sec += 1;    // 1 second timeout
    
    int retry;
    do {
        retry = 0;
        if( sem_timedwait( s->m_sem_lock, &timeout ) != 0 ) {
            if( errno == EINTR ) {
                retry = 1;
                // just interrupted by a signal, not really an error
            }
            else {
                LOG_ERROR( s, where, "sem_timedwait(lock) failed: %s", strerror(errno) );
                rv = MESSENGER_ERROR;
            }
        }
        else {
            int waiting_clients = 0;
            if( sem_getvalue( s->m_sem_notify, &waiting_clients ) != 0 ) {
                LOG_ERROR( s, where, "sem_getvalue(notify) failed: %s", strerror(errno) );
                rv = MESSENGER_ERROR;
            }
            else {
                for(;  waiting_clients < 1; waiting_clients++ ) {
                    if( sem_post( s->m_sem_notify ) != 0 ) {
                        LOG_ERROR( s, where, "sem_post(notify) failed: %s", strerror(errno) );
                        rv = MESSENGER_ERROR;
                        break;
                    }
                }
            }
            if( sem_post( s->m_sem_lock ) != 0 ) {
                LOG_ERROR( s, where, "sem_post(lock) failed: %s", strerror(errno) );
                rv = MESSENGER_ERROR;
            }
        }
    }
    while( retry );
    return rv;
}

messenger_status_t
messenger_server_break_mainloop( messenger_server_t* s )
{
    s->m_end = 1;
    // break if mainloop is in waitstate
    kill( getpid(), SIGALRM );
    return MESSENGER_OK;
}


messenger_status_t
messenger_server_mainloop( messenger_server_t*          s,
                           messenger_server_consumer_t  consumer,
                           void*                        consumer_data,
                           messenger_server_producer_t  producer,
                           void*                        producer_data,
                           messenger_periodic_t         periodic,
                           void*                        periodic_data )
{
    messenger_status_t rv = MESSENGER_OK;
    static const char* where = "ipc.messenger.server.mainloop";
    
    // --- install signal handlers ---------------------------------------------
    struct sigaction act, old_SIGTERM;
    memset( &act, 0, sizeof(act) );
    sigemptyset( &act.sa_mask );
    act.sa_handler = messenger_handler_SIGTERM;
    if( sigaction( SIGTERM, &act, &old_SIGTERM ) != 0 ) {
        LOG_ERROR( s, where, "sigaction( SIGTERM ) failed: %s", strerror(errno) );
        rv = MESSENGER_ERROR;
    }
    else {
        struct sigaction act, old_SIGALRM;
        memset( &act, 0, sizeof(act) );
        sigemptyset( &act.sa_mask );
        act.sa_handler = messenger_handler_SIGALRM;
        if( sigaction( SIGALRM, &act, &old_SIGALRM ) != 0 ) {
            LOG_ERROR( s, where, "sigaction( SIGALRM ) failed: %s", strerror(errno) );
            rv = MESSENGER_ERROR;
        }
        else {
            // --- unblock signals that we want --------------------------------
            sigset_t mask, old_sigset;
            sigemptyset( &mask );
            sigaddset( &mask, SIGTERM );
            sigaddset( &mask, SIGALRM );
            if( sigprocmask( SIG_UNBLOCK, &mask, &old_sigset ) != 0 ) { // pthread_sigprocmask?
                LOG_ERROR( s, where, "sigprocmask( unblock ) failed: %s", strerror(errno) );
                rv = MESSENGER_ERROR;
            }
            else {
                sem_post( s->m_sem_lock );  // open lock for business

                
                struct timespec next_periodic;
                clock_gettime( CLOCK_REALTIME, &next_periodic );
                next_periodic.tv_sec += 60;     // invoke periodic once a minute

                s->m_end = 0;
                periodic( periodic_data, 0 );
                while( (rv==MESSENGER_OK) && (exit_flag==0) && (s->m_end==0) ) {
                    
                    // --- wait for incomint message ---------------------------
                    if( sem_timedwait( s->m_sem_query, &next_periodic ) != 0 ) {
                        if( (errno == EINTR) || (errno == ETIMEDOUT) ) {
                            // not an error, we just got notified or timed out
                        }
                        else {
                            LOG_ERROR( s, where, "sem_timedwait(query) failed: %s", strerror(errno) );
                            rv = MESSENGER_ERROR;
                            break;  // break immediately out of loop.
                        }
                    }
                    else {
                        // --- got incoming message ----------------------------
                        if( msync( s->m_shmem_ptr, s->m_shmem_size, MS_SYNC ) != 0 ) {
                            LOG_ERROR( s, where, "msync(in) failed." );
                        }
                        else {
                            // query process and reply
                            
                            consumer( consumer_data, s->m_shmem_ptr, s->m_shmem_size, 1, 0 );
                            
                            int more = 0;
                            size_t bytes = 0;
                            producer( producer_data, &more, s->m_shmem_ptr, &bytes, s->m_shmem_size, 1 );

                            msync( s->m_shmem_ptr, bytes, MS_SYNC | MS_INVALIDATE );
                            sem_post( s->m_sem_reply );
                        }
                    }

                    // We have been notified...
                    if( s->m_notify ) {
                        rv = messenger_server_notified( s );
                    }

                    struct timespec now;
                    if( clock_gettime( CLOCK_REALTIME, &now ) != 0 ) {
                        LOG_ERROR( s, where, "clock_gettime(now) failed: %s", strerror(errno) );
                        rv = MESSENGER_ERROR;
                        break;  // break immediately out of loop.
                    }
                    else {
                        if( (now.tv_sec < next_periodic.tv_sec)
                                || ((now.tv_sec == next_periodic.tv_sec) && (now.tv_nsec < next_periodic.tv_nsec)))
                        {
                            LOG_INFO( s, where, "time for a new periodic invocation." );

                            periodic( periodic_data, 0 );
                            
                            next_periodic = now;
                            next_periodic.tv_sec += 60;
                        }
                    }
                }
                
                if( sigprocmask( SIG_SETMASK, &old_sigset, NULL ) != 0 ) { // pthread_sigprocmask?
                    LOG_ERROR( s, where, "sigprocmask( set ) restore failed: %s", strerror(errno) );
                    rv = MESSENGER_ERROR;
                }
            }
            if( sigaction( SIGALRM, &old_SIGALRM, NULL ) != 0 ) {
                LOG_ERROR( s, where, "sigaction( SIGALRM ) restore failed: %s", strerror(errno) );
                rv = MESSENGER_ERROR;
            }
        }
        if( sigaction( SIGTERM, &old_SIGTERM, NULL ) != 0 ) {
            LOG_ERROR( s, where, "sigaction( SIGTERM ) restore failed: %s", strerror(errno) );
            rv = MESSENGER_ERROR;
        }
    }
    
    return rv;
}
