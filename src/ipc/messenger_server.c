#include <fcntl.h>           /* For O_* constants */
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include "tinia/ipc/messenger.h"

#define LOG_WRAP( s, level, where, msg, ... ) \
do { \
    if( s->m_log_func != NULL ) { \
        s->m_log_func( s->m_log_data, level, where, msg, __VA_ARGS__ ); \
    } \
} while( 0 )
#define LOG_ERROR( s, where, msg, ... ) LOG_WRAP( s, 0, where, msg, __VA_ARGS__ )
#define LOG_WARN( s, where, msg, ... )  LOG_WRAP( s, 1, where, msg, __VA_ARGS__ )
#define LOG_INFO( s, where, msg, ... )  LOG_WRAP( s, 2, where, msg, __VA_ARGS__ )

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
