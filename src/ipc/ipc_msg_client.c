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
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h> // memset + strerror
#include <pthread.h>

#include <unistd.h>
// shmem stuff
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include "tinia/ipc/ipc_msg.h"
#include "ipc_msg_internal.h"


const size_t tinia_ipc_msg_client_t_sizeof = sizeof( tinia_ipc_msg_client_t );



int
tinia_ipc_msg_client_init( tinia_ipc_msg_client_t*   client,
                           const char*               jobid,
                           tinia_ipc_msg_log_func_t  log_f,
                           void*                     log_d  )
{
    static const char* who = "tinia.ipc.msg.client.init";
    char errnobuf[256];
    int rc, ret=0;
    
    client->shmem_name[0] = '\0';
    client->logger_f = log_f;
    client->logger_d = log_d;
    client->shmem_base = MAP_FAILED;
    client->shmem_total_size = 0;
    client->shmem_header_ptr = (tinia_ipc_msg_header_t*)MAP_FAILED;
    client->shmem_header_size = 0;
    client->shmem_payload_ptr = MAP_FAILED;
    client->shmem_payload_size = 0;
    
    if( ipc_msg_shmem_path( client->logger_f,
                            client->logger_d,
                            client->shmem_name,
                            sizeof(client->shmem_name),
                            jobid ) != 0 )
    {
        return -1;
    }

    if( ipc_msg_fake_shmem != 0 ) {
        rc = pthread_mutex_lock( &ipc_msg_fake_shmem_lock );
        if( rc != 0 ) {
            client->logger_f( client->logger_d, 0, who,
                              "pthread_mutex_lock( ipc_msg_fake_shmem_lock ) failed: %s",
                              ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
            ret = -1;
        }
        else {
            if( ipc_msg_fake_shmem_ptr == NULL ) {
                client->logger_f( client->logger_d, 0, who,
                                  "Failed to open fake shared memory '%s'.",
                                  client->shmem_name );
                tinia_ipc_msg_client_release( client );
                ret = -1;;
            }
            else {
                client->shmem_total_size = ipc_msg_fake_shmem_size;
                client->shmem_base = ipc_msg_fake_shmem_ptr;
                ipc_msg_fake_shmem_users++;
            }
            rc = pthread_mutex_unlock( &ipc_msg_fake_shmem_lock );
            if( rc != 0 ) {
                client->logger_f( client->logger_d, 0, who,
                                  "pthread_mutex_unlock( ipc_msg_fake_shmem_lock ) failed: %s",
                                  ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                ret = -1;
            }
        }
            
    }
    else {
        // --- open shared memory segment ------------------------------------------
        int fd = shm_open( client->shmem_name, O_RDWR, 0600 );
        if( fd < 0 ) {
            client->logger_f( client->logger_d, 0, who,
                              "Failed to open shared memory '%s': %s",
                              client->shmem_name,
                              ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
            ret = -1;
        }
        else {
            
            // --- determine size of shared memory segment -----------------------------
            struct stat fstat_buf;
            if( fstat( fd, &fstat_buf ) != 0 ) {
                client->logger_f( client->logger_d, 0, who,
                                  "Failed to determine size of shared memory '%s': %s",
                                  client->shmem_name,
                                  ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
                ret = -1;
            }
            else {
                client->shmem_total_size = fstat_buf.st_size;
                
                // --- map shared memory segment into process' address space ---------------
                client->shmem_base = mmap( NULL,
                                           fstat_buf.st_size,
                                           PROT_READ | PROT_WRITE,
                                           MAP_SHARED,
                                           fd,
                                           0 );
                if( client->shmem_base == MAP_FAILED ) {
                    client->logger_f( client->logger_d, 0, who,
                                      "Failed to map %ld bytes of shared memory '%s': %s",
                                      fstat_buf.st_size,
                                      client->shmem_name,
                                      ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
                    ret = -1;
                }
            }
            close( fd );
        }
        
    }
    if( ret == 0 ) {
        
        // --- update pointers and do some sanity checks ---------------------------    
        client->shmem_header_ptr = (tinia_ipc_msg_header_t*)client->shmem_base;
        client->shmem_header_size = client->shmem_header_ptr->header_size;
        client->shmem_payload_ptr = (char*)client->shmem_base + client->shmem_header_size;
        client->shmem_payload_size = client->shmem_header_ptr->payload_size;
        
        if( client->shmem_total_size != (client->shmem_header_size + client->shmem_payload_size) ) {
            client->logger_f( client->logger_d, 0, who,
                              "E: %lu != %lu + %lu!\n",
                              client->shmem_total_size,
                              client->shmem_header_size,
                              client->shmem_payload_size );
            ret = -1;
        }
    }
    
    if( ret != 0 ) {
        tinia_ipc_msg_client_release( client );
    }
    return ret;
}

int
tinia_ipc_msg_client_release( tinia_ipc_msg_client_t* client )
{
    static const char* who = "tinia.ipc.msg.client.release";
    char errnobuf[256];
    int ret=0;

    if( ipc_msg_fake_shmem != 0 ) {
        if ( pthread_mutex_lock( &ipc_msg_fake_shmem_lock ) != 0) {
            return -1;
        }

        ipc_msg_fake_shmem_users--;

        if ( pthread_mutex_unlock( &ipc_msg_fake_shmem_lock ) != 0) {
            return -1;
        }
    }
    else {
        if( client->shmem_base != MAP_FAILED ) {
            if( munmap( client->shmem_base, client->shmem_total_size ) != 0 ) {
                client->logger_f( client->logger_d, 0, who,
                                  "Failed to unmap shared memory '%s': %s",
                                  client->shmem_name,
                                  ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
                ret = -1;
            }
        }
    }
    // Note that MAP_FAILED is used to flag shared memory as "unmapped" or "unused", 
    // not necessarily that mapping has really failed.
    client->shmem_name[0] = '\0';
    client->shmem_base = MAP_FAILED;
    client->shmem_total_size = 0;
    client->shmem_header_ptr = (tinia_ipc_msg_header_t*)MAP_FAILED;
    client->shmem_header_size = 0;
    client->shmem_payload_ptr = MAP_FAILED;
    client->shmem_payload_size = 0;
    return ret;    
}

int
ipc_msg_client_signal_server( char* errnobuf,
                              size_t errnobuf_size,
                              tinia_ipc_msg_client_t* client )
{
    static const char* who = "tinia.ipc.msg.client.signal.server";
    
    client->shmem_header_ptr->server_event_predicate = 1;
    int rc = pthread_cond_signal( &client->shmem_header_ptr->server_event );
    if( rc != 0 ) {
        client->logger_f( client->logger_d, 0, who,
                          "pthread_cond_signal( client_event ): %s",
                          ipc_msg_strerror_wrap(rc, errnobuf, errnobuf_size ) );
        client->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
        return -2;
    }
    return 0;
}

int
ipc_msg_client_wait_server( char* errnobuf,
                            size_t errnobuf_size,
                            struct timespec* timeout,
                            tinia_ipc_msg_client_t* client )
{
    static const char* who = "tinia.ipc.msg.client.wait.server";
    
    client->shmem_header_ptr->client_event_predicate = 0;
    do {
        int rc = pthread_cond_timedwait( &client->shmem_header_ptr->client_event,
                                         &client->shmem_header_ptr->operation_lock,
                                         timeout );
        if( rc != 0 ) {
            client->logger_f( client->logger_d, 0, who,
                              "pthread_cond_timedwait( client_event ): %s",
                              ipc_msg_strerror_wrap(rc, errnobuf, errnobuf_size ) );
            tinia_ipc_msg_dump_backtrace( client->logger_f, client->logger_d );
            
            client->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
            ipc_msg_client_signal_server( errnobuf, errnobuf_size, client );
            if( rc == ETIMEDOUT ) {
                return -1;
            }
            else {
                return -2;
            }
        }
    } while( client->shmem_header_ptr->client_event_predicate == 0 );
    return 0;
}


int
ipc_msg_client_send( char* errnobuf,
                     size_t errnobuf_size,
                     struct timespec* timeout,
                     tinia_ipc_msg_client_t* client,
                     tinia_ipc_msg_producer_func_t producer, void* producer_data )
{
    static const char* who = "tinia.ipc.msg.client.send";

    // invariants: 
    // - transaction lock held
    // - operation lock held
    
    int ret = 0;
    int part, more;
    //struct timespec timeout;
    //clock_gettime( CLOCK_REALTIME, &timeout );
    //timeout.tv_sec += 5;
    
    for( part=0, more=1; (more==1) && (ret == 0); part++ ) {

        // --- unless first part, wait for server to finish --------------------
        if( part != 0 ) {
            if( (ret = ipc_msg_client_wait_server( errnobuf, errnobuf_size, timeout, client )) != 0 ) {
                break;
            }
        }
        
        // --- check that state is as expected (and not error) -----------------
        if( client->shmem_header_ptr->state != IPC_MSG_STATE_CLIENT_TO_SERVER ) {
            client->logger_f( client->logger_d, 0, who,
                              "Got state %d, expected state %d.",
                              client->shmem_header_ptr->state,
                              IPC_MSG_STATE_CLIENT_TO_SERVER );
            ret = -1;
            client->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
            ipc_msg_client_signal_server( errnobuf, errnobuf_size, client );
            break;
        }
        
        // --- produce message part --------------------------------------------
        size_t buffer_bytes;
        if( producer( producer_data,
                      &more,
                      (char*)client->shmem_payload_ptr,
                      &buffer_bytes,
                      client->shmem_payload_size,
                      part ) != 0 )
        {
            // --- error while producing message part --------------------------
            client->logger_f( client->logger_d, 0, who,
                              "Producer failed populating part %d.", part );
            ret = -1;
            client->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
            ipc_msg_client_signal_server( errnobuf, errnobuf_size, client );
            break;
        }
        client->shmem_header_ptr->part = part;
        client->shmem_header_ptr->more = more;
        client->shmem_header_ptr->bytes = buffer_bytes;
        
        // --- notify server about message part --------------------------------
        if( (ret = ipc_msg_client_signal_server( errnobuf, errnobuf_size, client )) != 0 ) {
            break;
        }
    }
    
#ifdef TRACE
    client->logger_f( client->logger_d, 2, who, "Sent %d parts to %s.", part, client->shmem_name );
#endif
    return ret;
}

int
ipc_msg_client_recv( char* errnobuf,
                     size_t errnobuf_size,
                     struct timespec* timeout,
                     tinia_ipc_msg_client_t* client,
                     tinia_ipc_msg_consumer_func_t consumer, void* consumer_data )
{
    static const char* who = "tinia.ipc.msg.client.recv";
    
    int do_wait_on_notification = 0;
    int ret = 0, part, rc;
 
    for( part=0; ret == 0 ; part++ ) {

        enum tinia_ipc_msg_state_t debug = client->shmem_header_ptr->state;
        
        // --- wait for server to be ready -------------------------------------
        if( (ret = ipc_msg_client_wait_server( errnobuf, errnobuf_size, timeout, client )) != 0 ) {
            client->logger_f( client->logger_d, 0, who, "server wait fail, part=%d, state=%d",
                              part, debug );
            
            break;
        }
        
        // --- check that state is as expected (and not error) -----------------
        if( client->shmem_header_ptr->state != IPC_MSG_STATE_SERVER_TO_CLIENT ) {
            client->logger_f( client->logger_d, 0, who,
                              "Got state %d, expected state %d.",
                              client->shmem_header_ptr->state,
                              IPC_MSG_STATE_SERVER_TO_CLIENT );
            ret = -1;
            client->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
            ipc_msg_client_signal_server( errnobuf, errnobuf_size, client );
            break;
        }
        
        // --- make sure that we're not out-of-sync ----------------------------
        if( client->shmem_header_ptr->part != part ) {
            client->logger_f( client->logger_d, 0, who,
                              "Got part %d, expected part %d.",
                              client->shmem_header_ptr->part, part );
            ret = -1;
            client->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
            ipc_msg_client_signal_server( errnobuf, errnobuf_size, client );
            break;
        }
        
        // consume message part
        int more = client->shmem_header_ptr->more;
        rc = consumer( consumer_data,
                       (char*)client->shmem_payload_ptr,
                       client->shmem_header_ptr->bytes,
                       part,
                       more );
        // longpolling?
        if( rc > 0 ) {
            do_wait_on_notification = 1;
        }
        // error? yes, tell server.
        else if( rc < 0 ) {
            ret = -1;
            client->shmem_header_ptr->state = IPC_MSG_STATE_ERROR;
            ipc_msg_client_signal_server( errnobuf, errnobuf_size, client );
            break;
        }
        
        // if this was the last part, finish.
        if( more == 0 ) {
            client->shmem_header_ptr->state = IPC_MSG_STATE_DONE;
        }
        
        // notify the server that it might start on the next part
        if( (ret = ipc_msg_client_signal_server( errnobuf, errnobuf_size, client ) ) != 0 ) {
            break;
        }
        if( more == 0 ) {
            break;
        }
    }
#ifdef TRACE
    client->logger_f( client->logger_d, 2, who, "Received %d parts from %s.", part+1, client->shmem_name );
#endif
    if( ret == 0 ) {
        ret = do_wait_on_notification;
    }
    return ret;
}


int
tinia_ipc_msg_client_sendrecv_by_name( const char*                    destination,
                                       tinia_ipc_msg_log_func_t       log_f,
                                       void*                          log_d,
                                       tinia_ipc_msg_producer_func_t  producer,
                                       void*                          producer_data,
                                       tinia_ipc_msg_consumer_func_t  consumer,
                                       void*                          consumer_data,
                                       int                            longpoll_timeout )
{
    static const char* who = "tinia.ipc.msg.client.sendrecv_by_name";
    int rv = -1;
    
    tinia_ipc_msg_client_t client;
    if( tinia_ipc_msg_client_init( &client, destination, log_f, log_d ) != 0 ) {
        log_f( log_d, 0, who, "Failed to open connection to '%s'", destination );
    }
    else {
        rv = tinia_ipc_msg_client_sendrecv( &client,
                                            producer, producer_data,
                                            consumer, consumer_data,
                                            longpoll_timeout );
        tinia_ipc_msg_client_release( &client );
    }
    return rv;    
}


int
tinia_ipc_msg_client_sendrecv( tinia_ipc_msg_client_t*        client,
                               tinia_ipc_msg_producer_func_t  producer,
                               void*                          producer_data,
                               tinia_ipc_msg_consumer_func_t  consumer,
                               void*                          consumer_data,
                               int                            longpoll_timeout )
{
    static const char* who = "tinia.ipc.msg.client.sendrecv";
    char errnobuf[256];
    int rc, ret = 0;

    struct timespec timeout, timeout_lp;

    if( clock_gettime( CLOCK_REALTIME, &timeout ) != 0 ) {
        client->logger_f( client->logger_d, 0, who,
                          "clock_gettime( CLOCK_REALTIME ): %s",
                          ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
        return -2;
    }
  
    // timeout for longpolling
    timeout_lp = timeout;
    timeout_lp.tv_sec += longpoll_timeout;
   
    // timeout for transaction lock
    timeout.tv_sec += 1;
    
    // --- make sure that we are the only client that interacts ----------------
    rc = pthread_mutex_timedlock( &client->shmem_header_ptr->transaction_lock,
                                  &timeout );
    if( rc != 0 ) {
        client->logger_f( client->logger_d, 0, who,
                          "pthread_mutex_timedlock( transaction_lock ): %s",
                          ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
        ret = -2;
    }
    else {
        do {
            ret = 0;

            // --- create timeout for this transaction -------------------------
            if( clock_gettime( CLOCK_REALTIME, &timeout ) != 0 ) {
                client->logger_f( client->logger_d, 0, who,
                                  "clock_gettime( CLOCK_REALTIME ): %s",
                                  ipc_msg_strerror_wrap(errno, errnobuf, sizeof(errnobuf) ) );
                ret = -2;
                break;
            }
            timeout.tv_sec += 1;

            // --- get control of shared memory buffer -------------------------
            rc = pthread_mutex_timedlock( &client->shmem_header_ptr->operation_lock,
                                          &timeout );
            if( rc != 0 ) {
                client->logger_f( client->logger_d, 0, who,
                                  "pthread_mutex_timedlock( operation_lock ): %s",
                                  ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                ret = -2;
            }
            else {

                // --- make sure that server is ready --------------------------
                while( client->shmem_header_ptr->state != IPC_MSG_STATE_READY ) {
                    int rc = pthread_cond_timedwait( &client->shmem_header_ptr->client_event,
                                                     &client->shmem_header_ptr->operation_lock,
                                                     &timeout );
                    if( rc != 0 ) {
                        if( rc == ETIMEDOUT ) {
                            ret = -1;
                        }
                        else {
                            ret = -2;
                        }
                        client->logger_f( client->logger_d, 0, who,
                                          "pthread_cond_timedwait( client_event ): %s",
                                          ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                        tinia_ipc_msg_dump_backtrace( client->logger_f, client->logger_d );
                        break;
                    }
                }
                
                if( ret == 0 ) {
                    // --- send query to server --------------------------------
                    client->shmem_header_ptr->state = IPC_MSG_STATE_CLIENT_TO_SERVER;
                    ret = ipc_msg_client_send( errnobuf,
                                               sizeof(errnobuf),
                                               &timeout,
                                               client,
                                               producer,
                                               producer_data );
                    // --- receive response from server ------------------------
                    if( ret == 0 ) {
                        ret = ipc_msg_client_recv( errnobuf,
                                                   sizeof(errnobuf),
                                                   &timeout,
                                                   client,
                                                   consumer,
                                                   consumer_data );
                    }
                }
                // --- release control of shared buffer ------------------------
                rc = pthread_mutex_unlock( &client->shmem_header_ptr->operation_lock );
                if( rc != 0 ) {
                    client->logger_f( client->logger_d, 0, who,
                                      "pthread_mutex_unlock( operation_lock ): %s",
                                      ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                    ret = -2;
                }
            }
            
            // --- longpolling -------------------------------------------------
            // If ipc_msg_client_recv returned 1, the callback functions have
            // indicated that we should longpoll. However, if longpoll timeout
            // is zero, we ignore this.
            if( ret > 0 ) {
                if( longpoll_timeout < 1 ) {
                    ret = 0;
                    break;  // no longpolling, just break out
                }
                client->logger_f( client->logger_d, 2, who, "Wait on notification from %s.", client->shmem_name );
                rc = pthread_cond_timedwait( &client->shmem_header_ptr->notification_event,
                                             &client->shmem_header_ptr->transaction_lock,
                                             &timeout_lp );
                if( rc != 0 ) {
                    if( rc == ETIMEDOUT ) {
                        ret = -1;
                        // this is likely to happen when there actually are no
                        // notifications, and is not an error

                        // client->logger_f( client->logger_d, 2, who,
                        //                   "pthread_cond_timedwait( notification_event ): %s",
                        //                   ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                        // tinia_ipc_msg_dump_backtrace( client->logger_f, client->logger_d );
                        client->logger_f( client->logger_d, 2, who, "Timed out waiting on notification from %s.", client->shmem_name );
                    }
                    else {
                        ret = -2;
                        client->logger_f( client->logger_d, 0, who,
                                          "pthread_cond_timedwait( notification_event ): %s",
                                          ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
                        tinia_ipc_msg_dump_backtrace( client->logger_f, client->logger_d );
                        
                    }
                }
                else {
                    client->logger_f( client->logger_d, 2, who, "Got notification from %s.", client->shmem_name );
                }
            }
        }
        while( ret > 0 );
        
        
        rc = pthread_mutex_unlock( &client->shmem_header_ptr->transaction_lock );
        if( rc != 0 ) {
            client->logger_f( client->logger_d, 0, who,
                              "pthread_mutex_unlock( transaction_lock ): %s",
                              ipc_msg_strerror_wrap(rc, errnobuf, sizeof(errnobuf) ) );
            ret = -2;
        }
    }
    return ret;
}


struct sendrecv_buffered_ctx
{
    const char* query;
    size_t      query_sent;
    size_t      query_size;
    char*       reply;
    size_t      reply_received;
    size_t      reply_buffer_size;
};

static
int
sendrecv_buffered_producer( void* data,
                            int* more,
                            char* buffer,
                            size_t* buffer_bytes,
                            const size_t buffer_size,
                            const int part )
{
    struct sendrecv_buffered_ctx* ctx = (struct sendrecv_buffered_ctx*)data;
    if( part == 0 ) {
        ctx->query_sent = 0;
    }
    size_t size = ctx->query_size - ctx->query_sent;
    if( buffer_size < size ) {
        size = buffer_size;
        *more = 1;  // we need more invocations
    }
    else {
        *more = 0;  // this is the last invocation
    }
    memcpy( buffer, ctx->query + ctx->query_sent, size );
    *buffer_bytes = size;
    
    ctx->query_sent += size;
    return 0;
}

static
int
sendrecv_buffered_consumer( void* data,
                            const char* buffer,
                            const size_t buffer_bytes,
                            const int part,
                            const int more )
{
    struct sendrecv_buffered_ctx* ctx = (struct sendrecv_buffered_ctx*)data;
    if( part == 0 ) {
        ctx->reply_received = 0;
    }

    if( ctx->reply_buffer_size < ctx->reply_received + buffer_bytes ) {
        // insufficiently sized buffer passed from caller
        return -1;
    }
    memcpy( ctx->reply + ctx->reply_received, buffer, buffer_bytes );

    ctx->reply_received += buffer_bytes;
    return 0;
}

int
ipc_msg_client_sendrecv_buffered( tinia_ipc_msg_client_t* client,
                                  const char* query, const size_t query_size,
                                  char* reply, size_t* reply_size, const size_t reply_buffer_size)
{
    struct sendrecv_buffered_ctx ctx;
    ctx.query             = query;
    ctx.query_sent        = 0;
    ctx.query_size        = query_size;
    ctx.reply             = reply;
    ctx.reply_received    = 0;
    ctx.reply_buffer_size = reply_buffer_size;
    
    int rv = tinia_ipc_msg_client_sendrecv( client,
                                      sendrecv_buffered_producer, &ctx,
                                      sendrecv_buffered_consumer, &ctx,
                                      0 );
    if( rv == 0 ) {
        *reply_size = ctx.reply_received;
        return 0;
    }
    else {
        *reply_size = 0;
        return -1;
    }
}


int
ipc_msg_client_sendrecv_buffered_by_name( const char*               destination,
                                          tinia_ipc_msg_log_func_t  logger_f,
                                          void*                     logger_d,
                                          const char*               query,
                                          const size_t              query_size,
                                          char*                     reply,
                                          size_t*                   reply_size,
                                          const size_t              reply_buffer_size )
{
    static const char* who = "tinia.ipc.msg.client.sendrecv_buffered_by_name";
    int rv = -1;
    
    tinia_ipc_msg_client_t client;
    if( tinia_ipc_msg_client_init( &client, destination, logger_f, logger_d ) != 0 ) {
        logger_f( logger_d, 0, who, "Failed to open connection to '%s'", destination );
    }
    else {
        struct sendrecv_buffered_ctx ctx;
        ctx.query             = query;
        ctx.query_sent        = 0;
        ctx.query_size        = query_size;
        ctx.reply             = reply;
        ctx.reply_received    = 0;
        ctx.reply_buffer_size = reply_buffer_size;
        
        int rv = tinia_ipc_msg_client_sendrecv( &client,
                                                sendrecv_buffered_producer, &ctx,
                                                sendrecv_buffered_consumer, &ctx,
                                                0 );
        tinia_ipc_msg_client_release( &client );
        if( rv == 0 ) {
            *reply_size = ctx.reply_received;
            return 0;
        }
        else {
            *reply_size = 0;
            return -1;
        }
    }
    return rv;
}

