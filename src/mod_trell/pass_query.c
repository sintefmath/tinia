#include <httpd.h>
#include <http_log.h>
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
#include <http_protocol.h>
#include <util_filter.h>
#include <unistd.h>
#include <time.h>
#include <apr_strings.h>
#include <stdarg.h>

#include "tinia/trell/trell.h"
#include "mod_trell.h"


int
trell_pass_query_get_renderlist( void*           data,
                                 size_t*         bytes_written,
                                 unsigned char*  buffer,
                                 size_t          buffer_size )
{
    trell_callback_data_t* cbd = (trell_callback_data_t*)data;

    tinia_msg_get_renderlist_t* msg = (tinia_msg_get_renderlist_t*)buffer;
    msg->msg.type = TRELL_MESSAGE_GET_RENDERLIST;
    memcpy( msg->session_id, cbd->m_dispatch_info->m_sessionid, TRELL_SESSIONID_MAXLENGTH );
    msg->session_id[ TRELL_SESSIONID_MAXLENGTH ] = '\0';
    
    memcpy( msg->key, cbd->m_dispatch_info->m_key, TRELL_KEYID_MAXLENGTH );
    msg->key[ TRELL_KEYID_MAXLENGTH ] = '\0';

    memcpy( msg->timestamp, cbd->m_dispatch_info->m_timestamp, TRELL_TIMESTAMP_MAXLENGTH );
    msg->timestamp[ TRELL_TIMESTAMP_MAXLENGTH ] = '\0';
    
    //trell_message_t* msg = (trell_message_t*)buffer;
    //msg->m_type = TRELL_MESSAGE_GET_RENDERLIST;
    //memcpy( msg->m_get_renderlist.m_session_id,
     //       cbd->m_dispatch_info->m_sessionid,
    //        TRELL_SESSIONID_MAXLENGTH );
    //memcpy( msg->m_get_renderlist.m_key,
    //        cbd->m_dispatch_info->m_key,
    //        TRELL_KEYID_MAXLENGTH );
    //memcpy( msg->m_get_renderlist.m_timestamp,
    //        cbd->m_dispatch_info->m_timestamp,
    //        TRELL_TIMESTAMP_MAXLENGTH );

    *bytes_written = sizeof(tinia_msg_get_renderlist_t);//TRELL_MESSAGE_GET_RENDERLIST_SIZE;
    return 0;
}

#if 0
int
trell_pass_query_get_exposedmodel( void*           data,
                                   size_t*         bytes_written,
                                   unsigned char*  buffer,
                                   size_t          buffer_size )
{
    trell_callback_data_t* cbd = (trell_callback_data_t*)data;
    
    trell_message_t* msg = (trell_message_t*)buffer;
    msg->m_type = TRELL_MESSAGE_GET_POLICY_UPDATE;
    msg->m_size = 0u;
    msg->m_get_model_update_payload.m_revision = cbd->m_dispatch_info->m_revision;
    strncpy( msg->m_get_model_update_payload.m_session_id,
             cbd->m_dispatch_info->m_sessionid,
             TRELL_SESSIONID_MAXLENGTH );
    msg->m_get_model_update_payload.m_session_id[ TRELL_SESSIONID_MAXLENGTH-1 ] = '\0';

    *bytes_written = msg->m_size + TRELL_MESSAGE_GET_POLICY_UPDATE_SIZE;
    return 0;
}
#endif

#if 0
int
trell_pass_query_update_state_xml( void*           data,
                                   size_t*         bytes_written,
                                   unsigned char*  buffer,
                                   size_t          buffer_size )
{
    trell_callback_data_t* cbd = (trell_callback_data_t*)data;

    // Create message
    tinia_msg_t* msg = (tinia_msg_t*)buffer;
    msg->type = TRELL_MESSAGE_UPDATE_STATE;

    *bytes_written = sizeof(*msg);
    
    // set up fetching of input
    apr_bucket_brigade* bb;
    apr_bucket* b;
    apr_status_t rv;

    bb = apr_brigade_create( cbd->m_r->pool, cbd->m_r->connection->bucket_alloc );

    
    // this code is funky (in a bad sense):
    int keep_going = 1;
    while(keep_going == 1) {
        apr_size_t free = buffer_size - sizeof(*msg);
        rv = ap_get_brigade( cbd->m_r->input_filters,
                             bb,
                             AP_MODE_READBYTES,
                             APR_BLOCK_READ,
                             free );
        if( rv != APR_SUCCESS ) {
            ap_log_rerror( APLOG_MARK, APLOG_CRIT, rv, cbd->m_r, "trell@pass_xml_query: ap_get_brigade failed" );
            return -1;
        }
        for( b = APR_BRIGADE_FIRST(bb); b!=APR_BRIGADE_SENTINEL(bb); b = APR_BUCKET_NEXT(b) ) {
            if( APR_BUCKET_IS_EOS(b) ) {
                keep_going = 0;
                break;
            }
            else if( APR_BUCKET_IS_METADATA(b) ) {
                continue;
            }

            apr_size_t bytes = 0;
            const char* buf = NULL;
            rv = apr_bucket_read( b,
                                  &buf,
                                  &bytes,
                                  APR_BLOCK_READ );

            
            if( rv != APR_SUCCESS ) {
                ap_log_rerror( APLOG_MARK, APLOG_CRIT, rv, cbd->m_r, "trell@pass_xml_query: apr_bucket_read failed" );
                return -1;
            }
            if( free < bytes ) {
                ap_log_rerror( APLOG_MARK, APLOG_CRIT, rv, cbd->m_r, "trell@pass_xml_query: multi-part message not yet supported." );
                return -1;
            }
            
            memcpy( (char*)msg + *bytes_written, buf, bytes );
            *bytes_written += bytes;
        }
    }
    
    apr_brigade_cleanup( bb );
    return 0;
}
#endif

int
trell_pass_query_msg_post( void*           data,
                           int*            more,
                           char*           buffer,
                           size_t*         bytes_written,
                           const size_t    buffer_size,
                           const int       part )
{
    trell_pass_query_msg_post_data_t* pass_func_data = (trell_pass_query_msg_post_data_t*)data;
    *bytes_written = 0;
    *more = 0;       // are we finished, or do we have more data to send?
    
    if( pass_func_data->r->connection->aborted ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, pass_func_data->r, "%s: connection aborted.", __func__ );
        return -1;
    }
    
    
    if( part == 0 ) {
        pass_func_data->message_offset = 0;
    }

    
    // --- copy message part (if anything left) into buffer --------------------
    if( pass_func_data->message_offset < pass_func_data->message_size ) {

        size_t bytes = pass_func_data->message_size - pass_func_data->message_offset; 
        if( buffer_size < bytes ) {
            bytes = buffer_size; // clamp to buffer size
            *more = 1;
        }
        memcpy( buffer,
                pass_func_data->message + pass_func_data->message_offset,
                bytes );
        pass_func_data->message_offset += bytes;
        *bytes_written += bytes;
        
        //ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, pass_func_data->r,
        //               "%s.pass_query_msg_post: wrote %d bytes of header [%d].",
        //               pass_func_data->r->handler, (int)bytes, *((int*)buffer) );
    }
    
    // --- copy data from HTTP POST if requested and any data present ----------
    if( /*(*bytes_written < buffer_size) &&*/ (pass_func_data->pass_post) ) {
        *more = 1;                               // always more until we see EOF
        size_t bytes = buffer_size - *bytes_written;   // bytes left in buffer
        
        apr_bucket_brigade* bb = apr_brigade_create( pass_func_data->r->pool,
                                                     pass_func_data->r->connection->bucket_alloc );
        
        apr_status_t rv = ap_get_brigade( pass_func_data->r->input_filters,
                                          bb,
                                          AP_MODE_READBYTES,
                                          APR_BLOCK_READ,
                                          bytes );
        if( rv != APR_SUCCESS ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, pass_func_data->r,
                           "%s.pass_query_msg_post: ap_get_brigade failed.",
                           pass_func_data->r->handler );
            return -1;
        }
        
        // run through buckets and check for EOF
        apr_bucket* e;
        for( e=APR_BRIGADE_FIRST(bb); e!=APR_BRIGADE_SENTINEL(bb); e=APR_BUCKET_NEXT(e) ) {
            if( APR_BUCKET_IS_EOS(e) ) {
                *more = 0;   // last iteration!
            }
        }
        
        // flatten buckets into buffer
        apr_size_t wrote = bytes;
        rv = apr_brigade_flatten( bb, buffer + *bytes_written, &wrote );
        if( rv != APR_SUCCESS ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, pass_func_data->r,
                           "%s.pass_query_msg_post: apr_brigade_flatten failed.", pass_func_data->r->handler );
            return -1;
        }
        *bytes_written += wrote;
    }
    //ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, pass_func_data->r,
    //               "%s.pass_query_msg_post: wrote a total of %d bytes, part=%d.",
    //               pass_func_data->r->handler, (int)(*bytes_written), part );
    return 0;
}