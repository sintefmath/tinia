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
#include <apr_buckets.h>
#include <stdarg.h>

#include "mod_trell.h"
#include "tinia/trell/trell.h"


int
trell_handle_get_script( trell_sconf_t           *sconf,
                         request_rec             *r,
                         trell_dispatch_info_t   *dispatch_info)
{
    tinia_msg_get_script_t query;
    query.msg.type = TRELL_MESSAGE_GET_SCRIPTS;
    
    trell_pass_query_msg_post_data_t pass_query_data;
    pass_query_data.sconf          = sconf;
    pass_query_data.r              = r;
    pass_query_data.dispatch_info  = dispatch_info;
    pass_query_data.message        = &query;
    pass_query_data.message_offset = 0;
    pass_query_data.message_size   = sizeof(query);
    pass_query_data.pass_post      = 0;

    tinia_pass_reply_data_t pass_reply_data;
    pass_reply_data.sconf         = sconf;
    pass_reply_data.r             = r;
    pass_reply_data.dispatch_info = dispatch_info;
    pass_reply_data.longpolling   = 0;
    pass_reply_data.brigade       = NULL;
    
    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "%s", __func__ );
    int rv = tinia_ipc_msg_client_sendrecv_by_name( dispatch_info->m_jobid,
                                                    trell_messenger_log_wrapper, r,
                                                    trell_pass_query_msg_post, &pass_query_data,
                                                    trell_pass_reply, &pass_reply_data,
                                                    0 );
    
    if( rv == 0 ) {
        return OK;
    }
    else if( rv == -1 ) {
        return HTTP_REQUEST_TIME_OUT;
    }
    else {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "%s: failed.", r->path_info );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
}

int
trell_handle_get_renderlist( trell_sconf_t*          sconf,
                             request_rec*            r,
                             trell_dispatch_info_t*  dispatch_info )
{
    tinia_msg_get_renderlist_t query;
    query.msg.type = TRELL_MESSAGE_GET_RENDERLIST;
    memcpy( &query.session_id, dispatch_info->m_sessionid, TRELL_SESSIONID_MAXLENGTH );
    query.session_id[TRELL_SESSIONID_MAXLENGTH] = '\0';
    memcpy( &query.key, dispatch_info->m_key, TRELL_KEYID_MAXLENGTH );
    query.key[TRELL_KEYID_MAXLENGTH] = '\0';
    memcpy( &query.timestamp, dispatch_info->m_timestamp, TRELL_TIMESTAMP_MAXLENGTH );
    query.timestamp[TRELL_TIMESTAMP_MAXLENGTH] = '\0';
    
    
    trell_pass_query_msg_post_data_t pass_query_data;
    pass_query_data.sconf          = sconf;
    pass_query_data.r              = r;
    pass_query_data.dispatch_info  = dispatch_info;
    pass_query_data.message        = &query;
    pass_query_data.message_offset = 0;
    pass_query_data.message_size   = sizeof(query);
    pass_query_data.pass_post      = 0;

    tinia_pass_reply_data_t pass_reply_data;
    pass_reply_data.sconf         = sconf;
    pass_reply_data.r             = r;
    pass_reply_data.dispatch_info = dispatch_info;
    pass_reply_data.longpolling   = 0;
    pass_reply_data.brigade       = NULL;

    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "%s", __func__ );
    int rv = tinia_ipc_msg_client_sendrecv_by_name( dispatch_info->m_jobid,
                                                    trell_messenger_log_wrapper, r,
                                                    trell_pass_query_msg_post, &pass_query_data,
                                                    trell_pass_reply, &pass_reply_data,
                                                    0 );
    
    if( rv == 0 ) {
        return OK;
    }
    else if( rv == -1 ) {
        return HTTP_REQUEST_TIME_OUT;
    }
    else {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "%s: failed.", r->path_info );
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

    tinia_msg_get_snapshot_t query;
    query.msg.type     = TRELL_MESSAGE_GET_SNAPSHOT;
    query.pixel_format = dispatch_info->m_pixel_format;
    query.width        = dispatch_info->m_width;
    query.height       = dispatch_info->m_height;
    memcpy( query.session_id, dispatch_info->m_sessionid, TRELL_SESSIONID_MAXLENGTH );
    query.session_id[TRELL_SESSIONID_MAXLENGTH] = '\0';
    memcpy( query.key, dispatch_info->m_key, TRELL_KEYID_MAXLENGTH );
    query.key[ TRELL_KEYID_MAXLENGTH ] = '\0';
    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "jny trell_handle_get_snapshot: %s: KEY=%s.", r->path_info, query.key );

    // create data for pass_query_msg_post
    trell_pass_query_msg_post_data_t pass_query_data;
    pass_query_data.sconf          = sconf;
    pass_query_data.r              = r;
    pass_query_data.dispatch_info  = dispatch_info;
    pass_query_data.message        = &query;
    pass_query_data.message_offset = 0;
    pass_query_data.message_size   = sizeof(query);
    pass_query_data.pass_post      = 0;

    trell_encode_png_state_t encode_png_state;
    encode_png_state.sconf         = sconf;
    encode_png_state.r             = r;
    encode_png_state.dispatch_info = dispatch_info;
    encode_png_state.width         = 0;
    encode_png_state.height        = 0;
    encode_png_state.buffer        = NULL;
    

    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "jny trell_handle_get_snapshot: viewer_key_list=%s", dispatch_info->m_viewer_key_list );


    int rv = tinia_ipc_msg_client_sendrecv_by_name( dispatch_info->m_jobid,
                                                    trell_messenger_log_wrapper, r,
                                                    trell_pass_query_msg_post, &pass_query_data,
                                                    trell_pass_reply_png, &encode_png_state,
                                                    0 );
    if( rv == 0 ) {
        return OK;
    }
    else if( rv == -1 ) {
        return HTTP_REQUEST_TIME_OUT;
    }
    else {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "%s: failed.", r->path_info );
        return HTTP_INTERNAL_SERVER_ERROR;
    }


#if 0
    rv = tinia_ipc_msg_client_sendrecv_by_name( dispatch_info->m_jobid,
                                                trell_messenger_log_wrapper, r,
                                                trell_pass_query_msg_post, &pass_query_data,
                                                trell_pass_reply_png, &encode_png_state,
                                                0 );
    if( rv == 0 ) {
        return OK;
    }
    else if( rv == -1 ) {
        return HTTP_REQUEST_TIME_OUT;
    }
    else {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "second bundle: %s: failed.", r->path_info );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
#endif


}


int
trell_handle_get_model_update( trell_sconf_t* sconf,
                                request_rec* r,
                                trell_dispatch_info_t*  dispatch_info )
{
    tinia_msg_get_exposed_model_t query;
    query.msg.type = TRELL_MESSAGE_GET_POLICY_UPDATE;
    query.revision = dispatch_info->m_revision;
    memcpy( query.session_id, dispatch_info->m_sessionid, TRELL_SESSIONID_MAXLENGTH );
    query.session_id[ TRELL_SESSIONID_MAXLENGTH ] = '\0';
    
    trell_pass_query_msg_post_data_t pass_query_data;
    pass_query_data.sconf          = sconf;
    pass_query_data.r              = r;
    pass_query_data.dispatch_info  = dispatch_info;
    pass_query_data.message        = &query;
    pass_query_data.message_offset = 0;
    pass_query_data.message_size   = sizeof(query);
    pass_query_data.pass_post      = 0;
    
    tinia_pass_reply_data_t pass_reply_data;
    pass_reply_data.sconf         = sconf;
    pass_reply_data.r             = r;
    pass_reply_data.dispatch_info = dispatch_info;
    pass_reply_data.longpolling   = 1;
    pass_reply_data.brigade       = NULL;
    
    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "[%d] %s", getpid(), __func__ );
    int rv = tinia_ipc_msg_client_sendrecv_by_name( dispatch_info->m_jobid,
                                                    trell_messenger_log_wrapper, r,
                                                    trell_pass_query_msg_post, &pass_query_data,
                                                    trell_pass_reply, &pass_reply_data,
                                                    30 );
    
    if( rv == 0 ) {
        return OK;
    }
    else if( rv == -1 ) {
        return HTTP_REQUEST_TIME_OUT;
    }
    else {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "%s: failed.", r->path_info );
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

    // create query message
    tinia_msg_update_exposed_model_t* qm = apr_palloc( r->pool, sizeof(*qm) );
    qm->msg.type = TRELL_MESSAGE_UPDATE_STATE;
    memcpy( qm->session_id, dispatch_info->m_sessionid, TRELL_SESSIONID_MAXLENGTH );
    qm->session_id[TRELL_SESSIONID_MAXLENGTH] = '\0';
    
    // create data for pass_query_msg_post
    trell_pass_query_msg_post_data_t qd;
    qd.sconf          = sconf;
    qd.r              = r;
    qd.dispatch_info  = dispatch_info;
    qd.message        = qm;
    qd.message_offset = 0;
    qd.message_size   = sizeof(*qm);
    qd.pass_post      = 1;

    // create data for trell_pass_reply
    tinia_pass_reply_data_t rd;
    rd.sconf = sconf;
    rd.r = r;
    rd.dispatch_info = dispatch_info;
    rd.longpolling = 0;
    rd.brigade = NULL;
    
    int rv = tinia_ipc_msg_client_sendrecv_by_name( dispatch_info->m_jobid,
                                                    trell_messenger_log_wrapper, r,
                                                    trell_pass_query_msg_post, &qd,
                                                    trell_pass_reply, &rd,
                                                    0 );

    if( rv == 0 ) {
        return OK;
    }
    else if( rv == -1 ) {
        return HTTP_REQUEST_TIME_OUT;
    }
    else {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "%s: failed.", r->path_info );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
}

// -----------------------------------------------------------------------------
int
trell_job_rpc_handle( trell_sconf_t* sconf,
                      request_rec*r,
                      xmlSchemaPtr schema,
                      const char* job,
                      trell_dispatch_info_t*  dispatch_info )
{
    
    // set up request config
    req_cfg_t* req_cfg = apr_palloc( r->pool, sizeof(*req_cfg) );
    req_cfg->m_schema = schema;
    ap_set_module_config( r->request_config, tinia_get_module(), req_cfg );
    // add validation filter
    ap_add_input_filter( "tinia_validate_xml", NULL, r, r->connection );
    
    if( r->method_number != M_POST ) {  // should be checked earlier
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

    tinia_msg_xml_t query;
    query.msg.type = TRELL_MESSAGE_XML;

    trell_pass_query_msg_post_data_t pass_query_data;
    pass_query_data.sconf          = sconf;
    pass_query_data.r              = r;
    pass_query_data.dispatch_info  = dispatch_info;
    pass_query_data.message        = &query;
    pass_query_data.message_offset = 0;
    pass_query_data.message_size   = sizeof(query);
    pass_query_data.pass_post      = 1;

    tinia_pass_reply_data_t pass_reply_data;
    pass_reply_data.sconf         = sconf;
    pass_reply_data.r             = r;
    pass_reply_data.dispatch_info = dispatch_info;
    pass_reply_data.longpolling   = 0;
    pass_reply_data.brigade       = NULL;
    

    int rv = tinia_ipc_msg_client_sendrecv_by_name( job,
                                                    trell_messenger_log_wrapper, r,
                                                    trell_pass_query_msg_post, &pass_query_data,
                                                    trell_pass_reply, &pass_reply_data,
                                                    0 );
    if( rv == 0 ) {
        return OK;
    }
    else if( rv == -1 ) {
        return HTTP_REQUEST_TIME_OUT;
    }
    else {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "%s: failed.", r->path_info );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
}
