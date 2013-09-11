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
trell_pass_query_get_snapshot( void*           data,
                               size_t*         bytes_written,
                               unsigned char*  buffer,
                               size_t          buffer_size )
{
    trell_callback_data_t* cbd = (trell_callback_data_t*)data;
    trell_message_t* msg = (trell_message_t*)buffer;

    msg->m_type = TRELL_MESSAGE_GET_SNAPSHOT;
    msg->m_get_snapshot.m_pixel_format = TRELL_PIXEL_FORMAT_BGR8;
    msg->m_get_snapshot.m_width = cbd->m_dispatch_info->m_width;
    msg->m_get_snapshot.m_height = cbd->m_dispatch_info->m_height;
    memcpy( msg->m_get_snapshot.m_session_id, cbd->m_dispatch_info->m_sessionid, TRELL_SESSIONID_MAXLENGTH );
    memcpy( msg->m_get_snapshot.m_key, cbd->m_dispatch_info->m_key, TRELL_KEYID_MAXLENGTH );

    *bytes_written = TRELL_MESSAGE_GET_POLICY_UPDATE_SIZE;
    return 0;
}

int
trell_pass_query_get_renderlist( void*           data,
                                 size_t*         bytes_written,
                                 unsigned char*  buffer,
                                 size_t          buffer_size )
{
    trell_callback_data_t* cbd = (trell_callback_data_t*)data;
    trell_message_t* msg = (trell_message_t*)buffer;
    
    msg->m_type = TRELL_MESSAGE_GET_RENDERLIST;
    memcpy( msg->m_get_renderlist.m_session_id,
            cbd->m_dispatch_info->m_sessionid,
            TRELL_SESSIONID_MAXLENGTH );
    memcpy( msg->m_get_renderlist.m_key,
            cbd->m_dispatch_info->m_key,
            TRELL_KEYID_MAXLENGTH );
    memcpy( msg->m_get_renderlist.m_timestamp,
            cbd->m_dispatch_info->m_timestamp,
            TRELL_TIMESTAMP_MAXLENGTH );

    *bytes_written = TRELL_MESSAGE_GET_RENDERLIST_SIZE;
    return 0;
}

int
trell_pass_query_get_scripts( void*           data,
                              size_t*         bytes_written,
                              unsigned char*  buffer,
                              size_t          buffer_size )
{
    //trell_callback_data_t* cbd = (trell_callback_data_t*)data;
    trell_message_t* msg = (trell_message_t*)buffer;
    msg->m_type = TRELL_MESSAGE_GET_SCRIPTS;
    *bytes_written = TRELL_MSGHDR_SIZE;
    return 0;
}


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

int
trell_pass_query_update_state_xml( void*           data,
                                   size_t*         bytes_written,
                                   unsigned char*  buffer,
                                   size_t          buffer_size )
{
    trell_callback_data_t* cbd = (trell_callback_data_t*)data;

    // Create message
    trell_message_t* msg = (trell_message_t*)buffer;
    msg->m_type = TRELL_MESSAGE_UPDATE_STATE;
    msg->m_size = 0u;

    // set up fetching of input
    apr_bucket_brigade* bb;
    apr_bucket* b;
    apr_status_t rv;

    bb = apr_brigade_create( cbd->m_r->pool, cbd->m_r->connection->bucket_alloc );

    
    int keep_going = 1;
    while(keep_going == 1) {
        apr_size_t free = buffer_size - TRELL_MESSAGE_UPDATE_STATE_SIZE - 1;
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
            
            memcpy( msg->m_update_state.m_xml + msg->m_size, buf, bytes );
            msg->m_size += bytes;
        }
    }
    *bytes_written = msg->m_size + TRELL_MESSAGE_UPDATE_STATE_SIZE;
    
    apr_brigade_cleanup( bb );
    return 0;
}
