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

#ifdef __cplusplus
extern "C" {
#endif

/** User-supplied callback that handles logging.
 *
 * \param[in] data     Optional data passed from callback supplier.
 * \param[in] level    Loglevel, 0 implies error, 1 implies warning, and 2
 *                     implies an informal message.
 * \param[in] who      Identifier of function that wants to log.
 * \param[in] message  Printf-formatted log message.
 * \param[in] ...      Arguments to log message.
 *
 */
typedef void (*tinia_ipc_msg_log_func_t)( void*        data,
                                          int          level,
                                          const char*  who,
                                          const char*  message, ... )
__attribute__((format(printf,4,5)))
;


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
                                                   char*                           buffer,
                                                   size_t                          buffer_bytes );


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





int
tinia_ipc_msg_client_sendrecv( tinia_ipc_msg_client_t*        client,
                               tinia_ipc_msg_producer_func_t  producer,
                               void*                          producer_data,
                               tinia_ipc_msg_consumer_func_t  consumer,
                               void*                          consumer_data,
                               int                            longpoll_timeout );


int
tinia_ipc_msg_client_sendrecv_by_name( const char*                    destination,
                                       tinia_ipc_msg_log_func_t       log_f,
                                       void*                          log_d,
                                       tinia_ipc_msg_producer_func_t  producer,
                                       void*                          producer_data,
                                       tinia_ipc_msg_consumer_func_t  consumer,
                                       void*                          consumer_data,
                                       int                            longpoll_timeout );


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
ipc_msg_client_sendrecv_buffered( tinia_ipc_msg_client_t* client,
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
                                          tinia_ipc_msg_log_func_t  logger_f,
                                          void*             logger_d,
                                          const char* query, const size_t query_size,
                                          char* reply, size_t* reply_size, const size_t reply_buffer_size);
        

int
tinia_ipc_msg_client_sendrecv_buffered_query_by_name( const char*                    destination,
                                                      tinia_ipc_msg_log_func_t       log_f,
                                                      void*                          log_d,
                                                      const char*                    query,
                                                      const size_t                   query_size,
                                                      tinia_ipc_msg_consumer_func_t  consumer,
                                                      void*                          consumer_data,
                                                      int                            longpoll_timeout );


// === SERVER PUBLIC API =======================================================

typedef struct tinia_ipc_msg_server_struct tinia_ipc_msg_server_t;

/** Create a new server.
 *
 * Note: This function must be invoked before any additional threads are
 * created, otherwise the signalmask cannot be set properly. 
 *
 */
tinia_ipc_msg_server_t*
ipc_msg_server_create( const char*       jobid,
                       tinia_ipc_msg_log_func_t  logger_f,
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
                         tinia_ipc_msg_periodic_func_t periodic, void* periodic_data,
                         tinia_ipc_msg_input_handler_func_t input_handler, void* input_handler_data,
                         tinia_ipc_msg_output_handler_func_t output_handler, void* output_handler_data );

#ifdef __cplusplus
}
#endif
