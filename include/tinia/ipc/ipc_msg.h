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

typedef void (*ipc_msg_logger_t)( void* data,
                                  int level,
                                  const char* who,
                                  const char* message, ... )
__attribute__((format(printf,4,5)))
;

typedef int (*ipc_msg_periodic_t)( void* data );

typedef int (*ipc_msg_consumer_t)( void* data,
                                   const char* buffer,
                                   const size_t buffer_bytes,
                                   const int iteration,
                                   const int more );

/** Callback that produces a message part-by-part.
 *
 * Messages are a sequence of one or more parts, where each part fit into a
 * fixed buffer.
 *
 * \param[in]  data          Optional data passed.
 * \param[out] more          Set to 0 if the current part is the last part of
 *                           the message, or 1 if this callback should be
 *                           invoked one more time.
 * \param[out] buffer Buffer into where to write the message part.
 * \param[out] buffer_bytes  The number of bytes written into buffer.
 * \param[in]  buffer_size   The maximum number of bytes that can be written
 *                           into buffer.
 * \param[in]  iteration     Enumerates the invocations of this callback for a
 *                           given message.
 *
 * \returns 0 on success, -1 on error, and if invoked on client-sude,
 * 1 indicates a success, but client should wait on a notification followed by a
 * new iteration of send and receive.
 */
typedef int (*ipc_msg_producer_t)( void* data,
                                   int* more,
                                   char* buffer,
                                   size_t* buffer_bytes,
                                   const size_t buffer_size,
                                   const int iteration );

/** Inspects the first part of a message and sets a message consumer.
 *
 * The server mainloop uses a pair of input and output handler callbacks to set
 * the specific message consumer and producer callbacks to be used for a given
 * message transaction.
 *
 * \param[out] consumer       Pointer to the message consumer callback.
 * \param[out] consumer_data  Optional data pointer passed to message consumer.
 * \param[in]  handler_data   Optional data passed from ipc_msg_server_mainloop.
 * \param[in]  buffer         First part of message contents.
 * \param[in]  buffer_bytes   First part of message byte size.
 *
 * \return 0 on success, -1 on error.
 */
typedef int (*ipc_msg_input_handler_t)( ipc_msg_consumer_t* consumer,
                                        void** consumer_data,
                                        void* handler_data,
                                        char* buffer,
                                        size_t buffer_bytes );

/** Sets a message producer.
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
typedef int (*ipc_msg_output_handler_t)( ipc_msg_producer_t* producer,
                                         void** producer_data,
                                         void* handler_data );


// === CLIENT PUBLIC API =======================================================

typedef struct client_struct tinia_ipc_msg_client_t;

extern const size_t tinia_ipc_msg_client_t_sizeof;

int
ipc_msg_client_init( tinia_ipc_msg_client_t*         client,
                     const char*       jobid,
                     ipc_msg_logger_t  logger_f,
                     void*             logger_d );

int
ipc_msg_client_release( tinia_ipc_msg_client_t* client );

int
ipc_msg_client_sendrecv( tinia_ipc_msg_client_t* client,
                         ipc_msg_producer_t producer, void* producer_data,
                         ipc_msg_consumer_t consumer, void* consumer_data,
                         int longpoll_timeout );

/** Send and receive a pair of messages using fixed buffers, no callbacks, and an open connection.
 *
 * \param[in]  client             Initialized client struct.
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
ipc_msg_client_sendrecv_buffered(tinia_ipc_msg_client_t* client,
                                  const char* query, const size_t query_size,
                                  char* reply, size_t* reply_size, const size_t reply_buffer_size);


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
                                          ipc_msg_logger_t  logger_f,
                                          void*             logger_d,
                                          const char* query, const size_t query_size,
                                          char* reply, size_t* reply_size, const size_t reply_buffer_size);
        
        

// === SERVER PUBLIC API =======================================================

typedef struct server_struct tinia_ipc_msg_server_t;

/** Create a new server.
 *
 * Note: This function must be invoked before any additional threads are
 * created, otherwise the signalmask cannot be set properly. 
 *
 */
tinia_ipc_msg_server_t*
ipc_msg_server_create( const char*       jobid,
                       ipc_msg_logger_t  logger_f,
                       void*             logger_d );

int
ipc_msg_server_delete( tinia_ipc_msg_server_t* server );

int
ipc_msg_server_wipe( const char* jobid );

int
ipc_msg_server_notify( tinia_ipc_msg_server_t* server );

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
                         ipc_msg_periodic_t periodic, void* periodic_data,
                         ipc_msg_input_handler_t input_handler, void* input_handler_data,
                         ipc_msg_output_handler_t output_handler, void* output_handler_data );
