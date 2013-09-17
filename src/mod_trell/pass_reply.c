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
#include <httpd.h>
#include <http_log.h>
#include <http_protocol.h>
#include <util_filter.h>
#include <unistd.h>
#include <time.h>
#include <apr_strings.h>
#include <stdarg.h>


#include "tinia/trell/trell.h"
#include "mod_trell.h"

// return 0 on success & finished, -1 failure, and 1 on longpoll wanted.
tinia_ipc_msg_status_t
trell_pass_reply_assert_ok( void* data,
                            unsigned char* buffer,
                            size_t offset,
                            size_t bytes,
                            int more )
{
    //trell_callback_data_t* cbd = (trell_callback_data_t*)data;
    trell_message_t* msg = (trell_message_t*)buffer;
    if( msg->m_type == TRELL_MESSAGE_OK ) {
        return MESSENGER_OK;
    }
    else {
        return MESSENGER_ERROR;
    }
}

tinia_ipc_msg_status_t
trell_pass_reply(void* data,
                  const char *buffer,
                  const size_t buffer_bytes,
                  const int first,
                  const int more )
{
    tinia_pass_reply_data_t* cbd = (tinia_pass_reply_data_t*)data;
    
    tinia_msg_t* msg = (tinia_msg_t*)buffer;

    size_t offset = 0;
    if( first ) {
        
        if( msg->type == TRELL_MESSAGE_OK ) {
            if( cbd->longpolling ) {
                return MESSENGER_TIMEOUT; // wait for notification
            }
            else {
                return MESSENGER_OK; // finished
            }
        }
        if( msg->type == TRELL_MESSAGE_OK ) {
            return MESSENGER_ERROR; // error
        }
        else if( msg->type == TRELL_MESSAGE_XML ) {
            ap_set_content_type( cbd->r, "application/xml" );
            offset = sizeof(*msg);
        }
        else if( msg->type == TRELL_MESSAGE_SCRIPT ) {
            ap_set_content_type( cbd->r, "application/javascript" );
            offset = sizeof(*msg);
        }
        else {
            ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, cbd->r,
                           "trell_pass_reply: Unexpected message type %d of size %d.",
                           (int)msg->type, (int)buffer_bytes );
            return MESSENGER_ERROR;
        }

        char* datestring = apr_palloc( cbd->r->pool, APR_RFC822_DATE_LEN );
        apr_rfc822_date( datestring, apr_time_now() );
        apr_table_setn( cbd->r->headers_out, "Last-Modified", datestring );
        apr_table_setn( cbd->r->headers_out, "Cache-Control", "no-cache" );
        
        cbd->brigade = apr_brigade_create( cbd->r->pool, cbd->r->connection->bucket_alloc );
    }
    
    size_t payload_bytes = buffer_bytes-offset;
    apr_bucket* b = apr_bucket_transient_create( buffer + offset,
                                                 payload_bytes,
                                                 cbd->brigade->bucket_alloc );

    APR_BRIGADE_INSERT_TAIL( cbd->brigade, b );
    if( !more ) {
        APR_BRIGADE_INSERT_TAIL( cbd->brigade, apr_bucket_eos_create( cbd->brigade->bucket_alloc ) );
    }

    apr_status_t rv = ap_pass_brigade( cbd->r->output_filters, cbd->brigade );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, cbd->r,
                       "trell_pass_reply: Failed to pass brigade." );
        return MESSENGER_ERROR;
    }
    
    if( more ) {
        rv = apr_brigade_cleanup( cbd->brigade );
        if( rv != APR_SUCCESS ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, cbd->r,
                           "trell_pass_reply: apr_brigade_cleanup failed. " );
            return MESSENGER_ERROR;
        }
    }
    else {
        rv = apr_brigade_destroy( cbd->brigade );
        if( rv != APR_SUCCESS ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, cbd->r,
                           "trell_pass_reply: apr_brigade_destroy failed. " );
            return MESSENGER_ERROR;
        }
    }
    return MESSENGER_OK;
}

