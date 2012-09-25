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
#include "mod_trell.h"


int
trell_send_txt_success( trell_sconf_t* sconf, request_rec* r )
{
    static const char* txt = "OK";
    ap_set_content_type( r, "text/plain");
    ap_set_content_length( r, strlen( txt ) );
    if( ap_rputs( txt, r ) < 0 ) {
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    else {
        return OK;
    }
}

int
trell_send_xml_success( trell_sconf_t*    sconf, request_rec*r )
{
    static const char* xml = "<?xml version=\"1.0\"><success/>";
    return trell_send_xml( sconf, r, xml, strlen(xml) );
}


int
trell_send_xml_failure( trell_sconf_t*    sconf, request_rec*r )
{
    static const char* xml = "<?xml version=\"1.0\"><failure/>";

    ap_set_content_type( r, "application/xml" );
    ap_set_content_length( r, strlen( xml ) );


    apr_bucket_brigade* bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );
    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_immortal_create( xml, strlen(xml), bb->bucket_alloc ) );
    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );

    apr_status_t rv = ap_pass_brigade( r->output_filters, bb );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Output error" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    else {
        return OK;
    }
}

int
trell_send_script( trell_sconf_t*   sconf,
                   request_rec*     r,
                   const char*      payload,
                   const size_t     payload_size )
{
    size_t size = payload_size;
    if( size > 0 && payload[size-1] == '\0' ) {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "Trimmed tailing zero from script." );
        size = size-1;
    }

    apr_bucket_brigade* bb;
    apr_bucket* b;
    char* datestring = apr_palloc( r->pool, APR_RFC822_DATE_LEN );
    apr_rfc822_date( datestring, apr_time_now() );

    ap_set_content_type( r, "application/javascript" );
    ap_set_content_length( r, size );
    apr_table_setn( r->headers_out, "Last-Modified", datestring );
    apr_table_setn( r->headers_out, "Cache-Control", "no-cache" );

    bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );
    b = apr_bucket_transient_create( payload, size, bb->bucket_alloc );

    APR_BRIGADE_INSERT_TAIL( bb, b );
    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );

    apr_status_t rv = ap_pass_brigade( r->output_filters, bb );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Output error" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    else {
        return OK;
    }
}


int
trell_send_xml( trell_sconf_t*   sconf,
                       request_rec*     r,
                       const char*      payload,
                       const size_t     payload_size )
{
    apr_time_t a = apr_time_now();
    size_t size = payload_size;
    if( size > 0 && payload[size-1] == '\0' ) {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "Trimmed tailing zero from XML." );
        size = size-1;
    }

    apr_bucket_brigade* bb;
    apr_bucket* b;
    char* datestring = apr_palloc( r->pool, APR_RFC822_DATE_LEN );
    apr_rfc822_date( datestring, apr_time_now() );

    ap_set_content_type( r, "application/xml" );
    ap_set_content_length( r, size );
    apr_table_setn( r->headers_out, "Last-Modified", datestring );
    apr_table_setn( r->headers_out, "Cache-Control", "no-cache" );

    bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );
    b = apr_bucket_transient_create( payload, size, bb->bucket_alloc );

    APR_BRIGADE_INSERT_TAIL( bb, b );
    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );

    apr_status_t rv = ap_pass_brigade( r->output_filters, bb );

    apr_time_t q = apr_time_now();
    float foo = ((float)(q-a)*(1000.0f/APR_USEC_PER_SEC));
    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "mod_trell: %d: sent xml, used %f ms", getpid(), foo );

    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Output error" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    else {
        return OK;
    }
}

int
trell_send_reply_xml( trell_sconf_t* sconf, request_rec* r, struct messenger* msgr )
{
    struct trell_message* msg = msgr->m_shmem_ptr;
    enum TrellMessageType message_type = msg->m_type;

    int retval = OK;
    if( message_type == TRELL_MESSAGE_XML ) {
        if( msg->m_size > 0 ) {

            if( retval == OK ) {
                apr_bucket_brigade* bb;
                apr_bucket* b;
                char* datestring = apr_palloc( r->pool, APR_RFC822_DATE_LEN );
                apr_rfc822_date( datestring, apr_time_now() );

                ap_set_content_type( r, "text/xml" );
                ap_set_content_length( r, msg->m_size-1 ); // msg->m_size includes null terminator
                apr_table_setn( r->headers_out, "Last-Modified", datestring );
                apr_table_setn( r->headers_out, "Cache-Control", "no-cache" );

                bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );
                b = apr_bucket_transient_create( msg->m_xml_payload, msg->m_size-1, bb->bucket_alloc );

                APR_BRIGADE_INSERT_TAIL( bb, b );
                APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );

                apr_status_t rv = ap_pass_brigade( r->output_filters, bb );
                if( rv != APR_SUCCESS ) {
                    ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Output error" );
                    retval = HTTP_INTERNAL_SERVER_ERROR;
                }
            }
        }
        else {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, OK, r,
                           "mod_trell: No path for message type %d", message_type );
            retval = HTTP_NO_CONTENT;
        }
        return retval;
    }
    else {

    }
    return HTTP_INTERNAL_SERVER_ERROR;
}
