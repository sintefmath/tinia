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

#include "tinia/trell/trell.h"
#include "mod_trell.h"

int
trell_pass_reply_xml_longpoll( void* data,
                               unsigned char* buffer,
                               size_t offset,
                               size_t bytes,
                               int more )
{
    trell_callback_data_t* cbd = (trell_callback_data_t*)data;
    request_rec* r = cbd->m_r;
    
    trell_message_t* msg = (trell_message_t*)buffer;
    if( msg->m_type == TRELL_MESSAGE_OK ) {
        // no updates, we should longpoll
        return 1;
    }
    else if( msg->m_type == TRELL_MESSAGE_XML ) {
        // wee, we got a reply
        
        const char* payload = msg->m_xml_payload;
        size_t payload_size = msg->m_size;

        // --- trim trailing zeros, if present ---------------------------------        
        int n = 0;
        while( payload_size > 0 && payload[ payload_size-1 ] == '\0' ) {
            payload_size--;
            n++;
        }
        if( n > 0 ) {
            ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "Trimmed %d trailing zeros from XML.", n );
        }

        // --- set http headers ------------------------------------------------        
        ap_set_content_type( r, "application/xml" );
        ap_set_content_length( r, payload_size );

        char* datestring = apr_palloc( r->pool, APR_RFC822_DATE_LEN );
        apr_rfc822_date( datestring, apr_time_now() );
        apr_table_setn( r->headers_out, "Last-Modified", datestring );

        apr_table_setn( r->headers_out, "Cache-Control", "no-cache" );

        // --- ship contents ---------------------------------------------------
        apr_bucket_brigade* bb;
        apr_bucket* b;
    
        bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );
        b = apr_bucket_transient_create( payload, payload_size, bb->bucket_alloc );

        APR_BRIGADE_INSERT_TAIL( bb, b );
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );

        apr_status_t rv = ap_pass_brigade( r->output_filters, bb );
        if( rv != APR_SUCCESS ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Failed to pass brigade." );
            return -1;
        }
        else {
            return 0;
        }
    }
    else {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "Unexpected message type %d of size %d.",
                       (int)msg->m_type, (int)msg->m_size );
        return -1;  // error
    }
}
