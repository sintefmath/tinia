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

#ifndef TINIA_IPC_MESSENGER_H
#define TINIA_IPC_MESSENGER_H
#include <semaphore.h>


#ifdef __cplusplus
extern "C" {
#endif


/** Client-side connection to a MessageBox. */


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
} tinia_ipc_msg_status_t;



typedef int (*ipc_msg_producer_t)( void* data, size_t* bytes_written, unsigned char* buffer, size_t buffer_size );
typedef int (*ipc_msg_consumer_t)( void* data, unsigned char* pointer, size_t offset, size_t bytes, int more  );

typedef void (*tinia_ipc_msg_logger_t)( void* data, int level, const char* who, const char* message, ... );

typedef tinia_ipc_msg_status_t (*messenger_server_consumer_t)( void* data,
                                                               const char* buffer,
                                                               const size_t buffer_bytes,
                                                               const int first,
                                                               const int more );
typedef tinia_ipc_msg_status_t (*messenger_server_producer_t)( void* data,
                                                               int* more,
                                                               char* buffer,
                                                               size_t* buffer_bytes,
                                                               const size_t buffer_size,
                                                               const int first );
typedef tinia_ipc_msg_status_t (*ipc_msg_periodic_t)( void* data, int seconds );

typedef struct tinia_ipc_msg_client_struct tinia_ipc_msg_client_t;
extern const size_t tinia_ipc_msg_client_t_sizeof;

const char*
messenger_strerror( tinia_ipc_msg_status_t error );

// -----------------------------------------------------------------------------

/** Initializes a new messenger struct. */
tinia_ipc_msg_status_t
tinia_ipc_msg_client_init( tinia_ipc_msg_client_t*  client,
                           const char*              message_box_id,
                           tinia_ipc_msg_logger_t   logger_func,
                           void*                    logger_data );

/** Releases resources associated with an initialized messenger. */
tinia_ipc_msg_status_t
tinia_ipc_msg_client_release( tinia_ipc_msg_client_t* client );


tinia_ipc_msg_status_t
tinia_ipc_msg_client_sendrecv_cb( ipc_msg_producer_t query, void* query_data,
                                  messenger_server_consumer_t reply, void* reply_data,
                                  tinia_ipc_msg_logger_t log, void* log_data,
                                  const char* message_box_id,
                                  int wait /* in seconds. */ );

tinia_ipc_msg_status_t
tinia_ipc_msg_client_sendrecv( const void* query, size_t query_size,
                               void* reply, size_t* reply_written, size_t reply_size,
                               tinia_ipc_msg_logger_t log, void* log_data,
                               const char* message_box_id,
                               int wait );






// -----------------------------------------------------------------------------

typedef struct tinia_ipc_msg_server_struct messenger_server_t;
extern const size_t messenger_server_t_sizeof;

tinia_ipc_msg_status_t
ipc_msg_server_init( messenger_server_t* e,
                         const char*         id,
                         tinia_ipc_msg_logger_t log_func,
                         void*               log_data );

tinia_ipc_msg_status_t
ipc_msg_server_release( messenger_server_t* e );

tinia_ipc_msg_status_t
ipc_msg_server_mainloop( messenger_server_t*          s,
                           messenger_server_consumer_t  consumer,
                           void*                        consumer_data,
                           messenger_server_producer_t  producer,
                           void*                        producer_data,
                           ipc_msg_periodic_t         periodic,
                           void*                        periodic_data );

/** Break the running mainloop of server. */
tinia_ipc_msg_status_t
ipc_msg_server_break( messenger_server_t* s );

/** Send a notification event through a server. */
tinia_ipc_msg_status_t
ipc_msg_server_notify( messenger_server_t* s );



#ifdef __cplusplus
}
#endif
#endif // TINIA_IPC_MESSENGER_H
