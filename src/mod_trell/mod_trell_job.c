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

// returns negative on error, zero on success, and 1 if more iterations are needed.


int
trell_handle_get_script( trell_sconf_t           *sconf,
                         request_rec             *r,
                         trell_dispatch_info_t   *dispatch_info)
{
    trell_callback_data_t cbd = { sconf, r, dispatch_info };
    switch( messenger_do_roundtrip( trell_pass_query_get_scripts, &cbd,
                                    trell_pass_reply_javascript, &cbd,
                                    trell_messenger_log_wrapper, r,
                                    dispatch_info->m_jobid,
                                    0 ) ) // no longpoll
    {
    case MESSENGER_OK:
        return OK; // everything ok.
    case MESSENGER_TIMEOUT:
        return HTTP_REQUEST_TIME_OUT;
    default:
        return HTTP_INTERNAL_SERVER_ERROR;
    }
}

int
trell_handle_get_renderlist( trell_sconf_t*          sconf,
                             request_rec*            r,
                             trell_dispatch_info_t*  dispatch_info )
{
    trell_callback_data_t cbd = { sconf, r, dispatch_info };
    switch( messenger_do_roundtrip( trell_pass_query_get_renderlist, &cbd,
                                    trell_pass_reply_xml_longpoll, &cbd,
                                    trell_messenger_log_wrapper, r,
                                    dispatch_info->m_jobid,
                                    0 ) ) // no longpoll
    {
    case MESSENGER_OK:
        return OK; // everything ok.
    case MESSENGER_TIMEOUT:
        return HTTP_REQUEST_TIME_OUT;
    default:
        return HTTP_INTERNAL_SERVER_ERROR;
    }    
}

int
trell_handle_get_snapshot( trell_sconf_t*          sconf,
                           request_rec*            r,
                           trell_dispatch_info_t*  dispatch_info )
{
    if( ( dispatch_info->m_width < 1 ) || ( dispatch_info->m_width > 2048  ) ||
        ( dispatch_info->m_width < 1 ) || ( dispatch_info->m_height > 2048 ) )
    {
        return HTTP_INSUFFICIENT_STORAGE;
    }
    trell_callback_data_t cbd = { sconf, r, dispatch_info };
    switch( messenger_do_roundtrip( trell_pass_query_get_snapshot, &cbd,
                                    trell_pass_reply_png, &cbd,
                                    trell_messenger_log_wrapper, r,
                                    dispatch_info->m_jobid,
                                    0 ) ) // no longpoll
    {
    case MESSENGER_OK:
        return OK; // everything ok.
    case MESSENGER_TIMEOUT:
        return HTTP_REQUEST_TIME_OUT;
    default:
        return HTTP_INTERNAL_SERVER_ERROR;
    }
}


int
trell_handle_get_model_update( trell_sconf_t* sconf,
                                request_rec* r,
                                trell_dispatch_info_t*  dispatch_info )
{
    trell_callback_data_t cbd = { sconf, r, dispatch_info };
    switch( messenger_do_roundtrip( trell_pass_query_get_exposedmodel, &cbd,
                                    trell_pass_reply_xml_longpoll, &cbd,
                                    trell_messenger_log_wrapper, r,
                                    dispatch_info->m_jobid,
                                    30 ) ) // longpoll for up to 30 secs
    {
    case MESSENGER_OK:
        return OK; // everything ok.
    case MESSENGER_TIMEOUT:
        return HTTP_REQUEST_TIME_OUT;
    default:
        return HTTP_INTERNAL_SERVER_ERROR;
    }
}


int
trell_handle_update_state( trell_sconf_t* sconf,
                           request_rec* r,
                           trell_dispatch_info_t*  dispatch_info )
{
    // check method
    if( r->method_number != M_POST ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: method != POST" );
        return HTTP_METHOD_NOT_ALLOWED;
    }
    // check if content type is correct
    const char* content_type = apr_table_get( r->headers_in, "Content-Type" );
    if( content_type == NULL ) {
        ap_log_rerror(  APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: No content type" );
        return HTTP_BAD_REQUEST;
    }

    // In case we get Content-Type: text/xml; encoding=foo.
    char* semicolon = strchr( content_type, ';' );
    if( semicolon != NULL ) {
        // Chop line at semicolon.
        *semicolon = '\0';
    }
    if( ! ( ( strcasecmp( "application/xml", content_type) == 0 ) ||
            ( strcasecmp( "text/xml", content_type ) == 0 ) ) )
    {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: Unsupported content-type '%s'", content_type );
        return HTTP_BAD_REQUEST;
    }


    trell_callback_data_t cbd = { sconf, r, dispatch_info };
    switch( messenger_do_roundtrip( trell_pass_query_update_state_xml, &cbd,
                                    trell_pass_reply_assert_ok, &cbd,
                                    trell_messenger_log_wrapper, r,
                                    dispatch_info->m_jobid,
                                    0 ) )
    {
    case MESSENGER_OK:
        return HTTP_NO_CONTENT; // everything ok.
    case MESSENGER_TIMEOUT:
        return HTTP_REQUEST_TIME_OUT;
    default:
        return HTTP_INTERNAL_SERVER_ERROR;
    }
}


// -----------------------------------------------------------------------------
int
trell_job_rpc_handle( trell_sconf_t* sconf,
                      request_rec*r,
                      xmlSchemaPtr schema,
                      const char* job )
{
    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "rpc %s.", r->path_info );

    // set up request config
    req_cfg_t* req_cfg = apr_palloc( r->pool, sizeof(*req_cfg) );
    req_cfg->m_schema = schema;
    ap_set_module_config( r->request_config, tinia_get_module(), req_cfg );
    // add validation filter
    ap_add_input_filter( "tinia_validate_xml", NULL, r, r->connection );

    
    apr_array_header_t* brigades = apr_array_make( r->pool, 10, sizeof(apr_bucket_brigade*) );
#if 0
    if( strcmp( job, sconf->m_master_id ) ) {

        int ret = trell_parse_xml( sconf, r, brigades, schema );
        if( ret != OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: Failure to get XML content" );
            return ret;
        }
    }
    else {

        int ret = trell_parse_xml( sconf, r, brigades, schema );
        if( ret != OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: Failure to get XML content" );
            return ret;
        }
    }
#endif

    struct messenger msgr;
    messenger_status_t mrv;

    mrv = messenger_init( &msgr, job, trell_messenger_log_wrapper, r );
    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_init('%s') failed: %s",  job, messenger_strerror( mrv ) );
        return HTTP_NOT_FOUND;
    }

    mrv = messenger_lock( &msgr );
    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_lock('%s') failed: %s",  job, messenger_strerror( mrv ) );

        mrv = messenger_free( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_free('%s') failed: %s",
                           job,
                           messenger_strerror( mrv ) );
        }
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    struct trell_message* msg = msgr.m_shmem_ptr;

    size_t size = trell_flatten_brigades_into_mem( sconf, r,
                                                   msg->m_xml_payload,
                                                   msgr.m_shmem_size - TRELL_MSGHDR_SIZE,
                                                   brigades );
    if( size == 0 ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: Failed to flatten brigades" );
        mrv = messenger_unlock( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_unlock('%s') failed: %s",  job, messenger_strerror( mrv ) );
        }
        mrv = messenger_free( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_free('%s') failed: %s",  job, messenger_strerror( mrv ) );
         }
        return HTTP_INTERNAL_SERVER_ERROR;
    }


    // --- post message ---
    msg->m_type = TRELL_MESSAGE_XML;
    msg->m_size = size;

    mrv = messenger_post( &msgr, size + TRELL_MSGHDR_SIZE );

    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_post('%s') failed: %s",  job, messenger_strerror( mrv ) );
        mrv = messenger_unlock( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_unlock('%s') failed: %s",  job, messenger_strerror( mrv ) );
        }
        mrv = messenger_free( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_free('%s') failed: %s",  job, messenger_strerror( mrv ) );
         }
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    // --- send reply to client ---
    int retcode = trell_send_reply_xml( sconf, r, &msgr );
    mrv = messenger_unlock( &msgr );
    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_unlock('%s') failed: %s",  job, messenger_strerror( mrv ) );
    }
    mrv = messenger_free( &msgr );
    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_free('%s') failed: %s",  job, messenger_strerror( mrv ) );
    }
    return retcode;
}
