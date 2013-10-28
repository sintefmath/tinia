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
#pragma once
#include <sys/types.h>
#include <tinia/ipc/ipc_msg.h>

enum tinia_ipc_msg_state_t
{
    IPC_MSG_STATE_READY,
    IPC_MSG_STATE_CLIENT_TO_SERVER,
    IPC_MSG_STATE_SERVER_TO_CLIENT,
    IPC_MSG_STATE_DONE,
    IPC_MSG_STATE_ERROR
};


/** The control data shared by client and server residing in the first page(s) of the shared memory.
 *
 *
 *  client                    server
 *                            take op lock
 *                            state = READY
 *                            wait on server event
 *  take tr lock
 *  take op lock
 *  state = CLIENT_TO_SERVER
 *  populate msg part 0
 *  signal server event
 *
 *
 */
typedef struct {
    /** True when all pthread-primitives has been initialized. */
    unsigned int        initialized;

    /** Number of bytes in the header-part of the shared memory. */
    size_t              header_size;

    /** Number of bytes in the payload-part of the shared memory. */
    size_t              payload_size;
    
    /** Lock for a sequence of operations.
     *
     * Makes sure that only one client may participate in a transaction. The
     * server may never take this lock, only a client. And a client may only
     * take this lock if it has not taken the operation lock.
     */
    pthread_mutex_t     transaction_lock;
    
    /** Lock for processing an operation.
     *
     * Gets sent back and forth between client and server during a transaction.
     * A client shall never take this lock without having succesfully taken
     * the transaction lock.
     */
    pthread_mutex_t     operation_lock;
    
    /** An operation is ready for the server to process.
     *
     * Condition is associated with operation_lock.
     */
    pthread_cond_t      server_event;
    int                 server_event_predicate;
    
    /** An operation is ready for the client to process.
     *
     * Condition is associated with operation_lock.
     */
    pthread_cond_t      client_event;
    int                 client_event_predicate;

    /** Allows clients to long-poll.
     *
     * Condition is associated with transaction_lock.
     */
    pthread_cond_t      notification_event;

    /** True when a server is listening. */
    int                 mainloop_running;

    /** Which part no of the current message is transmitted.
     *
     * Written by producer, verified by consumer.
     */
    int        part;
    
    /** More parts in current message?
     *
     * Used to notify the receiver that there will be another message part, used
     * by both client and server. 
     */    
    unsigned int        more;
    
    
    /** The number of bytes in the current message part. */
    size_t              bytes;

    enum tinia_ipc_msg_state_t    state;

} tinia_ipc_msg_header_t;

struct tinia_ipc_msg_client_struct {
    char                shmem_name[256];
    tinia_ipc_msg_log_func_t    logger_f;
    void*               logger_d;
    void*               shmem_base;
    size_t              shmem_total_size;
    tinia_ipc_msg_header_t*   shmem_header_ptr;
    size_t              shmem_header_size;
    void*               shmem_payload_ptr;
    size_t              shmem_payload_size;
};

struct tinia_ipc_msg_server_struct {
    pthread_t           thread_id;          ///< Thread id of the thread that initialized and runs the server.
    tinia_ipc_msg_log_func_t    logger_f;
    void*               logger_d;
    char*               shmem_name;
    void*               shmem_base;
    size_t              shmem_total_size;
    tinia_ipc_msg_header_t*   shmem_header_ptr;
    size_t              shmem_header_size;
    void*               shmem_payload_ptr;
    size_t              shmem_payload_size;
};

// === CLIENT AND SERVER COMMON API ============================================

// used for unit tests;
extern int              ipc_msg_fake_shmem;
extern pthread_mutex_t  ipc_msg_fake_shmem_lock;
extern void*            ipc_msg_fake_shmem_ptr;
extern size_t           ipc_msg_fake_shmem_size;
extern int              ipc_msg_fake_shmem_users;


int
ipc_msg_shmem_path( tinia_ipc_msg_log_func_t log_f, void* log_d, 
                    char* path, const size_t n, const char* jobid );

void
tinia_ipc_msg_dump_backtrace( tinia_ipc_msg_log_func_t log_f, void* log_d );

// === CLIENT INTERNAL API =====================================================


int
ipc_msg_client_signal_server( char* errnobuf,
                              size_t errnobuf_size,
                              tinia_ipc_msg_client_t* client );

int
ipc_msg_client_wait_server( char* errnobuf,
                            size_t errnobuf_size,
                            struct timespec* timeout,
                            tinia_ipc_msg_client_t* client );

int
ipc_msg_client_send( char* errnobuf,
                     size_t errnobuf_size,
                     struct timespec* timeout,
                     tinia_ipc_msg_client_t* client,
                     tinia_ipc_msg_producer_func_t producer, void* producer_data );

int
ipc_msg_client_recv( char* errnobuf,
                     size_t errnobuf_size,
                     struct timespec* timeout,
                     tinia_ipc_msg_client_t* client,
                     tinia_ipc_msg_consumer_func_t consumer, void* consumer_data );


// === SERVER INTERNAL API =====================================================

/** Thread body that waits for signals and acts upon them.
 * Note: This thread is designed to be canceable.
 */
void*
ipc_msg_server_signal_thread( void* data );

char*
ipc_msg_strerror_wrap( int errnum,
                       char* buf,
                       size_t buflen );

int
ipc_msg_server_recv( char* errnobuf,
                     size_t errnobuf_size,
                     tinia_ipc_msg_server_t* server,
                     tinia_ipc_msg_input_handler_func_t input_handler,
                     void* input_handler_data );

int
ipc_msg_server_send( char* errnobuf,
                            size_t errnobuf_size,
                            tinia_ipc_msg_server_t* server,
                            tinia_ipc_msg_output_handler_func_t output_handler,
                            void* output_handler_data );


int
ipc_msg_server_signal_client( char* errnobuf,
                              size_t errnobuf_size,
                              tinia_ipc_msg_server_t* server );

int
ipc_msg_server_wait_client( char* errnobuf,
                            size_t errnobuf_size,
                            tinia_ipc_msg_server_t* server );

int
ipc_msg_server_recv( char* errnobuf,
                     size_t errnobuf_size,
                     tinia_ipc_msg_server_t* server,
                     tinia_ipc_msg_input_handler_func_t input_handler,
                     void* input_handler_data );

int
ipc_msg_server_send( char* errnobuf,
                     size_t errnobuf_size,
                     tinia_ipc_msg_server_t* server,
                     tinia_ipc_msg_output_handler_func_t output_handler,
                     void* output_handler_data );

int
ipc_msg_server_mainloop_iteration( char* errnobuf,
                                   size_t errnobuf_size,
                                   struct timespec* periodic_timeout,
                                   tinia_ipc_msg_server_t* server,
                                   tinia_ipc_msg_periodic_func_t periodic, void* periodic_data,
                                   tinia_ipc_msg_input_handler_func_t input_handler, void* input_handler_data,
                                   tinia_ipc_msg_output_handler_func_t output_handler, void* output_handler_data );
