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

#ifndef MOD_TRELL_MESSENGER_H
#define MOD_TRELL_MESSENGER_H
#include <semaphore.h>


#ifdef __cplusplus
extern "C" {
#endif


/** Client-side connection to a MessageBox. */

  /* This example is outdated.
  * Basic usage:
  * \code
  * messenger m;
  * if( messenger_init( &m, <message_box_id> ) ) {
  *     if( messenger_lock(&m) ) {
  *         size_t msg_size = full size of message
  *         if( msg_size < m.m_shmem_size ) {
  *             trell_message* msg = reinterpret_cast<trell_message*>( m.m_shmem_ptr );
  *             // build query
  *             msg->m_type = TRELL_MESSAGE_xx
  *             msg->m_size = msg_size - TRELL_MSGHDR_SIZE; // size of payload
  *             // payload etc.
  *             if( messenger_post( &m, msg_size ) ) {
  *                 // msg now contains the reply
  *             }
  *         }
  *         messenger_unlock( &m );
  *     }
  *     messenger_free( &m );
  * }
  * \endcode
  */

typedef enum {
    MESSENGER_OK,
    MESSENGER_ERROR,
    MESSENGER_NULL,
    MESSENGER_SHMEM_LOCKED,
    MESSENGER_INVARIANT_BROKEN,
    MESSENGER_INVALID_MBOX,
    MESSENGER_OPEN_FAILED,
    MESSENGER_TIMEOUT,
    MESSENGER_INTERRUPTED,
    MESSENGER_INSUFFICIENT_MEMORY
} messenger_status_t;

struct messenger_header {
    size_t  m_size;
    int     m_more;
    int     m_error;
} __attribute__((aligned(16)));
typedef struct messenger_header messenger_header_t;

typedef int (*messenger_producer_t)( void* data, size_t* bytes_written, unsigned char* buffer, size_t buffer_size );
typedef int (*messenger_consumer_t)( void* data, unsigned char* pointer, size_t offset, size_t bytes, int more  );
typedef void (*messenger_logger_t)( void* data, int level, const char* who, const char* message, ... );

typedef messenger_status_t (*messenger_server_consumer_t)(void* data, const char* buffer, const size_t buffer_bytes, const int first, const int more );
typedef messenger_status_t (*messenger_server_producer_t)(void* data, int* more, char* buffer, size_t* buffer_bytes, const size_t buffer_size, const int first);
typedef messenger_status_t (*messenger_periodic_t)( void* data, int seconds );

typedef struct messenger
{
    int      m_has_lock;
    void*    m_shmem_ptr;
    size_t   m_shmem_size;
    sem_t*   m_lock;
    sem_t*   m_query;
    sem_t*   m_reply;
    sem_t*   m_notify;
    messenger_logger_t   m_logger_func;
    void*    m_logger_data;    
} messenger_t;

typedef struct messenger_server
{
    messenger_logger_t  m_log_func;
    void*               m_log_data;    
    
    /** Pointer to the shared memory of this message box. */
    void*           m_shmem_ptr;

    /** The size (in bytes) of the shared memory of this message box. */
    size_t          m_shmem_size;

    /** The system-wide name of the shared memory of this message box. */
    const char*     m_shmem_name;

    /** The semaphore that can lock this message box. */
    sem_t*          m_sem_lock;
    
    /** The system-wide name of the semaphore that can lock this message box. */
    const char*     m_sem_lock_name;
    /** The semaphore that signals a pending incoming message. */

    sem_t*          m_sem_query;

    /** The system-wide name of the semaphore that signals a pending incoming message. */
    const char*     m_sem_query_name;

    /** The semaphore that signals a pending reply in response to the incoming message. */
    sem_t*          m_sem_reply;

    /** The system-wide name of the semaphore that singals a pending reply. */
    const char*     m_sem_reply_name;
    
    /** Flags that a notify has occured. */
    volatile int    m_notify;
    
    volatile int    m_end;
    
    /** Used to notify jobs. */
    sem_t*          m_sem_notify;

    /** The system-wide name of the semaphore that signals a notify. */
    const char*     m_sem_notify_name;
    
    char            m_name[256];    // fixed size to avoid malloc/free
    
    

}messenger_server_t;




messenger_status_t
messenger_server_create( messenger_server_t* e,
                         const char*         id,
                         messenger_logger_t log_func,
                         void*               log_data );

messenger_status_t
messenger_server_destroy( messenger_server_t* e );

messenger_status_t
messenger_server_mainloop( messenger_server_t*          s,
                           messenger_server_consumer_t  consumer,
                           void*                        consumer_data,
                           messenger_server_producer_t  producer,
                           void*                        producer_data,
                           messenger_periodic_t         periodic,
                           void*                        periodic_data );

messenger_status_t
messenger_server_break_mainloop( messenger_server_t* s );

messenger_status_t
messenger_server_notify( messenger_server_t* s );


messenger_status_t
messenger_do_roundtrip_cb( messenger_producer_t query, void* query_data,
                           messenger_consumer_t reply, void* reply_data,
                           messenger_logger_t log, void* log_data,
                           const char* message_box_id,
                           int wait /* in seconds. */ );

messenger_status_t
messenger_do_roundtrip( const void* query, size_t query_size,
                        void* reply, size_t* reply_written, size_t reply_size,
                        messenger_logger_t log, void* log_data,
                        const char* message_box_id,
                        int wait );



const char*
messenger_strerror( messenger_status_t error );

/** Initializes a new messenger struct.
  *
  * \param m               The messenger struct to initialize.
  * \param message_box_id  The id of the target message box, which is usually
  *                        either the id of the job or trell_master.
  *
  * \note As this function accesses system-wide resources, it is important that
  *       this is cleaned up, which is done by calling messenger_free on
  *       successfully initialized messengers.
  *
  * \note The message box is not locked until messenger_lock is invoked. Hence,
  *       it safe to sit on an initialized messenger for some time.
  *
  * \returns 1 on success, 0 on failure.
  */
messenger_status_t
messenger_init( struct messenger* m,
                const char* message_box_id,
                void   (*logger_func)( void *data, int level, const char* who, const char* msg, ... ),
                void*    logger_data );

/** Releases resources associated with an initialized messenger.
  *
  * \param m  The messenger struct of which resources are to be released.
  */
messenger_status_t
messenger_free( struct messenger* m );

/** Get an exclusive lock on the target message box.
  *
  * \param m  An initialized but not yet locked messenger.
  *
  * \note Since this lock is exclusive, it is important to send the message and
  *       release the lock as fast as possible to allow other processes to
  *       access the message box.
  *
  */
//messenger_status_t
//messenger_lock( struct messenger* m );

//messenger_status_t
//messenger_wait_for_notification( struct messenger* m, int wait_seconds );


/** Send a message to the message box and wait for a reply.
  *
  * \param m  An initialized and locked messenger.
  * \param size  The full size of the message, in bytes.
  */
//messenger_status_t
//messenger_post( struct messenger*m, size_t size );

/** Release the exclusive lock on a message box.
  *
  * \param m An initialized and locked messenger, that may have sent a message.
  */
//messenger_status_t
//messenger_unlock( struct messenger* m );




#ifdef __cplusplus
}
#endif
#endif // MOD_TRELL_MESSENGER_H
