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
int
trell_pass_reply_assert_ok( void* data,
                            unsigned char* buffer,
                            size_t offset,
                            size_t bytes,
                            int more )
{
    //trell_callback_data_t* cbd = (trell_callback_data_t*)data;
    tinia_msg_t* msg = (tinia_msg_t*)buffer;
    if( msg->type == TRELL_MESSAGE_OK ) {
        return 0;   // ok
    }
    else {
        return -1;  // error
    }
}




int
trell_pass_reply( void*         data,
                  const char*   buffer,
                  const size_t  buffer_bytes,
                  const int     part,
                  const int     more )
{
    tinia_pass_reply_data_t* cbd = (tinia_pass_reply_data_t*)data;
    
    tinia_msg_t* msg = (tinia_msg_t*)buffer;

    size_t offset = 0;
    if( part == 0 ) {
        cbd->bytes_sent = 0;
        
        if( msg->type == TRELL_MESSAGE_OK ) {
            if( cbd->longpolling ) {
                return 1;               // wait for notification.
            }
            else {
                return 0;               // ok
            }
        }
        if( msg->type == TRELL_MESSAGE_OK ) {
            return -1;                  // error
        }
        else if( msg->type == TRELL_MESSAGE_XML ) {
            ap_set_content_type( cbd->r, "application/xml" );
            offset = sizeof(tinia_msg_xml_t);
        }
        else if( msg->type == TRELL_MESSAGE_SCRIPT ) {
            ap_set_content_type( cbd->r, "application/javascript" );
            offset = sizeof(*msg);
        }
        else {
            ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, cbd->r,
                           "trell_pass_reply: Unexpected message type %d of size %d (%s).",
                           (int)msg->type, (int)buffer_bytes, cbd->r->path_info );
            return -1;                  // error
        }

        char* datestring = apr_palloc( cbd->r->pool, APR_RFC822_DATE_LEN );
        apr_rfc822_date( datestring, apr_time_now() );
        apr_table_setn( cbd->r->headers_out, "Last-Modified", datestring );
        apr_table_setn( cbd->r->headers_out, "Cache-Control", "no-cache" );
        
        cbd->brigade = apr_brigade_create( cbd->r->pool, cbd->r->connection->bucket_alloc );
    }

    if( offset < buffer_bytes ) {
        size_t payload_bytes = buffer_bytes-offset;
        apr_bucket* b = apr_bucket_transient_create( buffer + offset,
                                                     payload_bytes,
                                                     cbd->brigade->bucket_alloc );
        cbd->bytes_sent += payload_bytes;
        APR_BRIGADE_INSERT_TAIL( cbd->brigade, b );
    }    

    if( !more ) {
        APR_BRIGADE_INSERT_TAIL( cbd->brigade, apr_bucket_eos_create( cbd->brigade->bucket_alloc ) );
    }

    apr_status_t rv = ap_pass_brigade( cbd->r->output_filters, cbd->brigade );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, cbd->r,
                       "trell_pass_reply: Failed to pass brigade." );
        return -1;                  // error
    }
    
    if( more ) {
        rv = apr_brigade_cleanup( cbd->brigade );
        if( rv != APR_SUCCESS ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, cbd->r,
                           "trell_pass_reply: apr_brigade_cleanup failed. " );
            return -1;                  // error
        }
    }
    else {
        rv = apr_brigade_destroy( cbd->brigade );
        if( rv != APR_SUCCESS ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, cbd->r,
                           "trell_pass_reply: apr_brigade_destroy failed. " );
            return -1;                  // error
        }
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, rv, cbd->r,
                       "%s: sent %ld bytes of %s. ", __func__, cbd->bytes_sent, cbd->r->content_type );
    }
    return 0;   // ok
}

