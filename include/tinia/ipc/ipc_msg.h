#pragma once
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

// === CALLBACK TYPES ==========================================================
#include "ipc_util.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup ipc_msg Interprocess messaging
 *
 * The tinia interprocess messaging allows the apache webserver to communicate
 * with jobs, and jobs to communicate with each other, a capability a job gets
 * by being controlled by an \ref tinia::trell::IPCController subclass.
 *
 * Clients and servers
 * -------------------
 * 
 * Messaging is carried out between a _client_ and a _server_. When a subclass
 * of \ref tinia::trell::IPCController controls a job, it also creates a
 * messaging server, and the main loop waits for messages on this server. Any
 * client can connect to this server, but only one client at a time. All jobs as
 * well as the apache webserver module create clients to communicate with
 * each other, through the messaging server.
 *
 * A client is tied to a specific server. When the client is initiated, a shared
 * memory segment owned by the server is mapped into the client's address space.
 * This memory segment contains, in addition to a communication buffer, some
 * locks and condition variables used for communication.
 *
 * The client initiates the communication, which consists of one or more
 * request-response-pairs. Processing of such a request-response pair is atomic
 * in the sense that a single client has exclusive ownership of the server
 * during that processing. The client sends the request, and the server provides
 * a response. The actual contents of these messages are irrelevant for the
 * messaging code, only the message size and its role as a request or a
 * response matters.
 *
 * Producers, consumers, and handlers
 * ----------------------------------
 *
 * Since a fixed buffer is used for communication and message size is arbitrary,
 * the messages must be split into one or more parts that fit into this buffer.
 * The burden of splitting the messages is put on the code that uses this API,
 * as that code may do something clever to avoid extra copies. This buffer is 
 * at least \ref TINIA_IPC_MSG_PART_MIN_BYTES, and hence, one can safely assume
 * that a message smaller than this doesn't need to be split into parts.
 * However, note that message parts doesn't _need_ to be of at least of this
 * size, it is just a guarantee for the buffer size.
 *
 * Splitting and merging of messages is handled by _producers_ and _consumers_,
 * which are callbacks with the signatures \ref tinia_ipc_msg_producer_func_t
 * and \ref tinia_ipc_msg_consumer_func_t respectively. These are called in
 * lockstep, first the producer is invoked for part 0, then the consumer is
 * invoked for part 0, then the producer for part 1, consumer for part 1, and so
 * on, until the producer indicates that the last part has been produced, or an
 * error has occured. The producer and consumer is invoked in different
 * processes writing and reading from a shared memory segment.
 *
 * To avoid having to create a single pair of producers and consumers that can
 * handle all types of messages, one specifies a pair of handlers (\ref
 * tinia_ipc_msg_input_handler_func_t and
 * \ref tinia_ipc_msg_output_handler_func_t) that are callbacks that are used
 * to set specific producers and consumers for a given content. That is, when
 * the server gets the first part of a message, the input handler is invoked
 * with that part. The input handler may use the contents of that message part
 * to specify an appropriate consumer, and that message part as well as all
 * subsequent parts are then passed to that consumer. Further, when a reply is
 * to be formed, the output handler is invoked to specify which producer that
 * should create the reply message.
 *
 * Notification and long-polling
 * -----------------------------
 *
 * A client may also wait for a given time to be notified by a server. The
 * server is free for other clients while a client is waiting. This
 * functionality is used to implement long-polling.
 *
 * On the server side, notification is invoked using \ref ipc_msg_server_notify,
 * which can be invoked from any thread, including the mainloop-thread (which
 * usually is a result of an update from a client).
 *
 * On the client side, waiting for notification is triggered by the consumer. If
 * the consumer returns 1, \ref tinia_ipc_msg_client_sendrecv does not
 * immediately return after handling a single pair of query-response messages,
 * but waits for a notification. If a notification happens before the wait
 * timeout, a new pair of query-response is handled, and so on until the
 * timeout. Specifying a zero timeout disables any waiting, guaranteeing a
 * single pair of query-response-messages.
 *
 * Server execution flow
 * ---------------------
 *
 * - Invoke \ref ipc_msg_server_create to initialize a server. This function
 *   must be called before any extra threads are created.
 *   - This creates a shared memory segment used for communication, and
 *     initializes some concurrency primitives inside this segment.
 * - Create other threads and do init code.
 * - Invoke \ref ipc_msg_server_mainloop to start listening. 
 *   - If a new query message arrives:
 *     - Invoke input handler to determine consumer.
 *     - Let consumer process all parts.
 *     - Invoke output handler to determine producer.
 *     - Let producer produce until it is finished.
 *   - If a notification:
 *     - Wake all clients waiting for a notification.
 *   - If mainloop break is requested:
 *     - Break out of loop and return.
 *   - Invoke \ref ipc_msg_server_delete to clean up
 *
 * Client execution flow
 * ---------------------
 *
 * - Each time the client wants to send one or more messages to the server:
 * - Either (elaborate):
 *   - Allocate \ref tinia_ipc_msg_client_t_sizeof bytes of memory
 *   - Invoke \ref tinia_ipc_msg_client_init on this memory, passing along the
 *     id of the destination server.
 *   - Invoke \ref tinia_ipc_msg_client_sendrecv one or more times.
 *   - Invoke \ref tinia_ipc_msg_client_release
 * - Or (simple):
 *   - Invoke \ref tinia_ipc_msg_client_sendrecv_by_name (which does the actions
 *     outlined above for you).
 *
 * @{
 *
 */

/** Minimum size for the buffer that holds message parts.
 *
 * It can be safely assumed that the buffer that is used to transmit message
 * parts between the client and server is of this size, and that messages
 * smaller than this size doesn't need to be split.
 *
 * \note This compile-time constant _might_ change, and thus, assumming it is of
 * at least a given fixed size warrants an assertion.
 */
#define TINIA_IPC_MSG_PART_MIN_BYTES 4096


/** User-supplied callback invoked every now and then by the server mainloop.
 *
 * \param[in] data   Optional data passed from callback supplier.
 *
 */
typedef int (*tinia_ipc_msg_periodic_func_t)( void* data );


/** User-supplied callback that consumes a message part-by-part.
 *
 * Messages are a sequence of one or more parts, where each part fit into a
 * fixed buffer. This callback is invoked once per message part.
 *
 * \warning In the case of long-polling, several message transactions take
 * place inside a single send-receive call. Thus, in this case, the callback
 * must initialize state when invoked with part=0.
 *
 * \param[in] data          Optional data passed from callback supplier.
 * \param[in] buffer        Buffer that contains the message part.
 * \param[in] buffer_bytes  Number of bytes in this message part.
 * \param[in] part          Enumerates the message parts.
 * \param[in] more          True if there is more parts of this message.
 *
 * \return 0 on success, -1 on error, and if invoked on client-side, 1 indicates
 * a success, but client should wait on a notification followed by a new
 * iteration of send and receive (used to implement long-polling).
 *
 */
typedef int (*tinia_ipc_msg_consumer_func_t)( void*         data,
                                              const char*   buffer,
                                              const size_t  buffer_bytes,
                                              const int     part,
                                              const int     more );


/** User-supplied callback that produces a message part-by-part.
 *
 * Messages are a sequence of one or more parts, where each part fit into a
 * fixed buffer. This callback is invoked once per message part.
 *
 * \warning In the case of long-polling, several message transactions take
 * place inside a single send-receive call. Thus, in this case, the callback
 * must initialize state when invoked with part=0.
 *
 * \param[in]  data          Optional data passed from callback supplier.
 * \param[out] more          Set to 0 if the current part is the last part of
 *                           the message, or 1 if this callback should be
 *                           invoked one more time.
 * \param[out] buffer Buffer into where to write the message part.
 * \param[out] buffer_bytes  The number of bytes written into buffer.
 * \param[in]  buffer_size   The maximum number of bytes that can be written
 *                           into buffer.
 * \param[in]  part          Enumerates the message parts.
 *
 * \return 0 on success, -1 on error.
 *
 */
typedef int (*tinia_ipc_msg_producer_func_t)( void*         data,
                                              int*          more,
                                              char*         buffer,
                                              size_t*       buffer_bytes,
                                              const size_t  buffer_size,
                                              const int     part );


/** User-supplied callback that inspects the first part of a message and sets a message consumer callback.
 *
 * The server mainloop uses a pair of input and output handler callbacks to set
 * the specific message consumer and producer callbacks to be used for a given
 * message transaction.
 *
 * \param[out] consumer       Pointer to the message consumer callback.
 * \param[out] consumer_data  Optional data pointer passed to message consumer.
 * \param[in]  handler_data   Optional data passed from callback supplier.
 * \param[in]  buffer         First part of message contents.
 * \param[in]  buffer_bytes   First part of message byte size.
 *
 * \return 0 on success, -1 on error.
 */
typedef int (*tinia_ipc_msg_input_handler_func_t)( tinia_ipc_msg_consumer_func_t*  consumer,
                                                   void**                          consumer_data,
                                                   void*                           handler_data,
                                                   const char*                     buffer,
                                                   const size_t                    buffer_bytes );


/** User-supplied callback that sets a message producer callback.
 *
 * The server mainloop uses a pair of input and output handler callbacks to set
 * the specific message consumer and producer callbacks to be used for a given
 * message transaction.
 *
 * \param[out] producer       Pointer to the producer callback.
 * \param[out] producer_data  Optional data that will be passed to producer.
 * \param[in]  handler_data   Optional data passed from ipc_msg_server_mainloop.
 *
 * \returns 0 on success, -1 on error.
 */
typedef int (*tinia_ipc_msg_output_handler_func_t)( tinia_ipc_msg_producer_func_t*  producer,
                                                    void**                          producer_data,
                                                    void*                           handler_data );


// === CLIENT PUBLIC API =======================================================

typedef struct tinia_ipc_msg_client_struct tinia_ipc_msg_client_t;

extern const size_t tinia_ipc_msg_client_t_sizeof;

/** Initializes a new message client.
 *
 * \param[inout] client       User-supplied buffer that contains the client
 *                            struct, the bytesize of this struct is
 *                            tinia_ipc_msg_client_t_sizeof.
 * \param[in]    destination  Job id of destination server.
 * \param[in]    logger_f     Callback that handles log messages.
 * \param[in]    logger_d     Optional data passed to logger callback.
 *
 * \return 0 on success, a negative value on error.
 */
int
tinia_ipc_msg_client_init( tinia_ipc_msg_client_t*   client,
                           const char*               jobid,
                           tinia_ipc_msg_log_func_t  log_f,
                           void*                     log_d );


/** Releases resources of a message client.
 *
 * \param[in] client  Buffer that contains the client struct.
 *
 * \return 0 on success, a negative value on error.
 */
int
tinia_ipc_msg_client_release( tinia_ipc_msg_client_t* client );


/** Send a query and process a reply on an initialized client.
 *
 * \param[in] client            Initialized client structure.
 * \param[in] producer          Callback to message producer function.
 * \param[in] producer_data     Optional data passed to the message producer
 *                              function.
 * \param[in] consumer          Callback to message consumer function.
 * \param[in] consumer_data     Optional data passed to the message consumer
 *                              function.
 * \param[in] longpoll_timeout  Maximum number of seconds to spend inside this
 *                              function waiting for a notification. Passing
 *                              zero disables waiting.
 * \return 0 on success, a negative value on an error.
 */
int
tinia_ipc_msg_client_sendrecv( tinia_ipc_msg_client_t*        client,
                               tinia_ipc_msg_producer_func_t  producer,
                               void*                          producer_data,
                               tinia_ipc_msg_consumer_func_t  consumer,
                               void*                          consumer_data,
                               int                            longpoll_timeout );

/** Set up a client and end a query and process a reply.
 *
 * Wraps \ref tinia_ipc_msg_client_sendrecv inside
 * \ref tinia_ipc_msg_client_init and \ref tinia_ipc_msg_client_release.
 *
 * \param[in] destination       Job id of destination server.
 * \param[in] producer          Callback to message producer function.
 * \param[in] producer_data     Optional data passed to the message producer
 *                              function.
 * \param[in] consumer          Callback to message consumer function.
 * \param[in] consumer_data     Optional data passed to the message consumer
 *                              function.
 * \param[in] longpoll_timeout  Maximum number of seconds to spend inside this
 *                              function waiting for a notification. Passing
 *                              zero disables waiting.
 * \return 0 on success, a negative value on an error.
 */
int
tinia_ipc_msg_client_sendrecv_by_name( const char*                    destination,
                                       tinia_ipc_msg_log_func_t       log_f,
                                       void*                          log_d,
                                       tinia_ipc_msg_producer_func_t  producer,
                                       void*                          producer_data,
                                       tinia_ipc_msg_consumer_func_t  consumer,
                                       void*                          consumer_data,
                                       int                            longpoll_timeout );


/** Open connection and send and receive a pair of messages using fixed buffers and no callbacks.
 *
 * \param[in]  destination        Where to open the connection.
 * \param[in]  query              Buffer that contains the query message.
 * \param[in]  query_size         Byte size of the query message.
 * \param[out] reply              Buffer into which the reply message will be written
 * \param[out] reply_size         Byte size of the reply message.
 * \param[in]  reply_buffer_size  Size of reply message buffer.
 *
 * \note A singe buffer can safely be used as both the query and reply buffer
 * simultaneously.
 */
int
ipc_msg_client_sendrecv_buffered_by_name( const char* destination,
                                          tinia_ipc_msg_log_func_t  logger_f,
                                          void*             logger_d,
                                          const char* query, const size_t query_size,
                                          char* reply, size_t* reply_size, const size_t reply_buffer_size);
        

// === SERVER PUBLIC API =======================================================

typedef struct tinia_ipc_msg_server_struct tinia_ipc_msg_server_t;


/** Create a new server.
 *
 * \param jobid     Id of the server.
 * \param logger_f  Callback used for logging.
 * \param logger_d  Optional data passed to logger callback.
 *
 * \warning This function must be invoked before any additional threads are
 * created, otherwise the signalmask cannot be set properly. 
 *
 * \warning Only one server should be created per process.
 */
tinia_ipc_msg_server_t*
ipc_msg_server_create( const char*       jobid,
                       tinia_ipc_msg_log_func_t  logger_f,
                       void*             logger_d );


/** Clean up and release resources of an existing server.
 *
 * \param[in] server  Pointer to an existing server.
 *
 * \return 0 on success, or a negative value on failure.
 */
int
ipc_msg_server_delete( tinia_ipc_msg_server_t* server );


/** Wipes the shared memory segment of a server.
 *
 * Used to clean up after jobs that has crashed and not released all of its
 * resources.
 *
 * \param jobid     Id of the server.
 * \param logger_f  Callback used for logging.
 * \param logger_d  Optional data passed to logger callback.
 *
 * \return 0 on success, or a negative value on failure.
 */
int
ipc_msg_server_wipe(tinia_ipc_msg_log_func_t log_f, void *log_d, const char* jobid );


/** Wake all clients waiting for notification.
 *
 * Wakes all the clients that are currently waiting for notification.
 *
 * If invoked in the same thread as the thread running the mainloop, waiting
 * clients are notified immediatly. Otherwise, it first waits for any currently
 * active clients to finish. This behaviour is to avoid that a client misses a
 * notification in the moment between a query was processed and a client starts
 * to wait for notifications.
 *
 * \param[in] server               Pointer to initialized server struct.
 *
 * \return 0 on success, or a negative value on failure.
 */
int
ipc_msg_server_notify( tinia_ipc_msg_server_t* server );


/** Break and return from the mainloop.
 *
 * Make the mainloop stop running and return control to the function that
 * invoked the mainloop.
 *
 * If this function is invoked in the same thread as the thread running the
 * mainloop, a flag is set such that the mainloop will break and return in the
 * next iteration. Otherwise, it first waits for any currently active clients
 * to finish, then setting the flag and signals the mainloop thread.
 *
 * \param[in] server               Pointer to initialized server struct.
 *
 * \return 0 on success, or a negative value on failure.
 */
int
ipc_msg_server_mainloop_break( tinia_ipc_msg_server_t* server );


/** Run message server mainloop.
 *
 * Runs the message server mainloop, invoking the pair of input and output
 * handlers for each message transaction.
 *
 * \warning This function must be invoked in the same thread that created the
 * server structure.
 *
 * \param[in] server               Pointer to initialized server struct.
 * \param[in] periodic             Callback that is invoked every now and then,
 *                                 intended for maintenance functions.
 * \param[in] periodic_data        Optional data passed to the periodic function.
 * \param[in] input_handler        Callback that is invoked to provide a
 *                                 message consumer that can consume the
 *                                 current message.
 * \param[in] input_handler_data   Optional data passed to the input handler.
 * \param[in] output_handler       Callback that is invoked to provide a
 *                                 message producer that can produce an
 *                                 reply message to the message that was
 *                                 consumed.
 * \param[in] output_handler_data  Optional data passed to the output handler.
 *
 */
int
ipc_msg_server_mainloop( tinia_ipc_msg_server_t* server,
                         tinia_ipc_msg_periodic_func_t periodic, void* periodic_data,
                         tinia_ipc_msg_input_handler_func_t input_handler, void* input_handler_data,
                         tinia_ipc_msg_output_handler_func_t output_handler, void* output_handler_data );

/** @} */

#ifdef __cplusplus
}
#endif
