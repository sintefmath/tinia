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
#include <ctype.h>
#include <apr_strings.h>
#include <apr_lib.h>
#include "mod_trell.h"

/** Convert a path string into an array of strings delimited by '/' */
static
apr_array_header_t*
trell_tokenize_path( apr_pool_t* pool, const char* string )
{
    const char* delimiters = "/";

    // create new string that we can play with
    char* my_string = apr_pstrdup( pool, string );

    // create an array that holds pointers into my_string
    apr_array_header_t* ret = apr_array_make( pool, 0, sizeof(my_string) );

    // tokenize path
    char* last; // state for apr_strtok
    char* token = apr_strtok( my_string, delimiters, &last );
    while( token ) {
        APR_ARRAY_PUSH( ret, char*) = token;
        token = apr_strtok( NULL, delimiters, &last );
    }

    return ret;
}

/** Convert an argument string into an hash.
 *
 * \note The HTTP protocol allows multiple entries with identical keys, but we
 *       disallow it.
 */
static
apr_hash_t*
trell_tokenize_args( apr_pool_t* pool, const char* string )
{
    const char* delimiter = "&";

    apr_hash_t* ret = apr_hash_make( pool );
    if( string == NULL ) {
        return ret; // no arguments
    }

    // create new string that we can play with
    char* my_string = apr_pstrdup( pool, string );

    char* last; // state for apr_strtok
    char* pair = apr_strtok( my_string, delimiter, &last );
    while( pair != NULL ) {
        char* eq = NULL;
        char* t;
        for( t = pair; *t; ++t ) {
            if( *t == '+' ) {   // convert + to spaces
                *t = ' ';
            }
            else if( eq == NULL && *t == '=' ) {    // found equal sign
                eq = t;
            }
        }
        if( eq ) {
            *eq++ = '\0';
            ap_unescape_url( pair );
            ap_unescape_url( eq );
        }
        else {
            eq = "";
            ap_unescape_url( pair );
        }
        apr_hash_set( ret, pair, APR_HASH_KEY_STRING, apr_pstrdup( pool, eq ) );

        // iterate forward
        pair = apr_strtok( NULL, delimiter, &last );
    }
    return ret;
}

static
int
trell_check_and_copy_id( request_rec*r,
                         const char* component,
                         const char* what,
                         char* dst, const char* string, int max_length )
{
    int i;
    if( string == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "%s: %s/%s is NULL.",
                       r->handler, component, what  );
        return 0;
    }
    for( i=0; i<max_length; i++ ) {
        char c = string[i];
        dst[i] = c;
        if( c == '\0' ) {
            if( i == 0 ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "%s: %s/%s is empty string.",
                               r->handler, component, what  );
                return 0;
            }
            return 1;
        }
        if( !(apr_isalnum ( c ) || c == '_' ) ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "%s: %s/%s contains illegal characters.",
                           r->handler, component, what  );
            return 0;
        }
    }
    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                   "%s: %s/%s is too long.",
                   r->handler, component, what );
    return 0;
}

static
int
trell_hash_strncpy( request_rec* r, char* dst, apr_hash_t* args, const char* key, int max_length )
{
    const char* string = apr_hash_get( args, key, APR_HASH_KEY_STRING );
    if( string == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "%s: args[%s]: no value.", r->handler, key );
        return 0;
    }

    int i;
    for( i=0; i<max_length; i++ ) {
        char c = string[i];
        dst[i] = c;
        if( c == '\0' ) {
            if( i == 0 ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "%s: args[%s]: empty string.", r->handler, key );
                return 0;
            }
            return 1;
        }
    }
    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                   "%s: args[%s]: string too long.", r->handler, key );
    return 0;
}


static int
trell_hash_atoi( request_rec* r,
                 const char* component,
                 const char* request,
                 int* result, apr_hash_t* args, const char* key, int required )
{
    const char* string = apr_hash_get( args, key, APR_HASH_KEY_STRING );
    if( string == NULL ) {
        if( required ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "%s: %s/%s: args[%s]: no value.",
                           r->handler, component, request, key );
            return 0;
        }
        else {
            *result = 0;
            return 1;
        }
    }
    apr_off_t value;
    char* end;
    apr_status_t rv = apr_strtoff( &value, string, &end, 10 );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r,
                       "%s: %s/%s: args[%s]: parsing of number failed (string='%s').",
                       r->handler, component, request, key, string );
        return 0;
    }
    *result = value;
    if( *end != '\0' ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r,
                       "%s: %s/%s: args[%s]: garbage after value (string='%s').",
                       r->handler, component, request, key, string );
        return 0;
    }
    return 1;
}

// path is /component[/jobid/sessionid]/action?args
//
// action is one of:
// - rpc.xml
// - getExposedModelUpdate.xml
// - updateState.xml
// - snapshot.png
// - snapshot.txt
// - getRenderList.xml
// - getScript.js
// args is some of
// - revision=ddddd
// - width=dddd
// - height=dddd
//


int
trell_decode_path_info( trell_dispatch_info_t* dispatch_info, request_rec *r )
{
    dispatch_info->m_component = TRELL_COMPONENT_NONE;
    dispatch_info->m_request = TRELL_REQUEST_NONE;
    dispatch_info->m_mod_action = TRELL_MOD_ACTION_NONE;
    dispatch_info->m_pixel_format = TRELL_PIXEL_FORMAT_UNDEFINED;
    dispatch_info->m_jobid[0] = '\0';
    dispatch_info->m_sessionid[0] = '\0';
    dispatch_info->m_key[0] ='\0';
    dispatch_info->m_timestamp[0] = '\0';
    dispatch_info->m_revision = 0;
    dispatch_info->m_base64 = 0;
    dispatch_info->m_width = 0;
    dispatch_info->m_height = 0;

    int i;
    int o = -1;

    if( r->path_info == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "%s: path_info is NULL.", r->handler );
        return HTTP_NOT_FOUND;
    }

    // --- convert path info to an array of strings ----------------------------
    apr_array_header_t* path_items = trell_tokenize_path( r->pool, r->path_info );

    // --- find start of query and component to use ----------------------------
    // we discard any intial tokens that we do not recognize.

    for( i=0; (i<path_items->nelts) && (o<0); i++ ) {
        char* item = APR_ARRAY_IDX( path_items, i, char*);
        if( apr_strnatcmp( item, "mod" ) == 0 ) {
            dispatch_info->m_component = TRELL_COMPONENT_OPS;
            o = i;
        }
        else if( apr_strnatcmp( item, "master" ) == 0 ) {
            dispatch_info->m_component = TRELL_COMPONENT_MASTER;
            o = i;
        }
        else if( apr_strnatcmp( item, "job" ) == 0 ) {
            dispatch_info->m_component = TRELL_COMPONENT_JOB;
            o = i;
        }
    }
    if( o < 0 ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "%s: path '%s' does not contain any component.",
                       r->handler, r->path_info );
        return HTTP_NOT_FOUND;
    }
    char* component = APR_ARRAY_IDX( path_items, o, char*);; // used for logging
    o = o+1;

    // --- if job component, determine job id ----------------------------------
    if( dispatch_info->m_component == TRELL_COMPONENT_JOB ) {
        if( trell_check_and_copy_id( r, component, "job id", dispatch_info->m_jobid,
                                     (o < path_items->nelts ? APR_ARRAY_IDX( path_items, o, char*) : NULL ),
                                     TINIA_IPC_JOBID_MAXLENGTH-1 ) == 0 )
        {
            return HTTP_BAD_REQUEST;
        }
        o = o+1;    // next token
    }

    // --- if job path, determine session id -----------------------------------
    if( dispatch_info->m_component == TRELL_COMPONENT_JOB ) {
        if( trell_check_and_copy_id( r, component, "session id", dispatch_info->m_sessionid,
                                     (o < path_items->nelts ? APR_ARRAY_IDX( path_items, o, char*) : NULL ),
                                     TRELL_SESSIONID_MAXLENGTH-1 ) == 0 )
        {
            return HTTP_BAD_REQUEST;
        }
        o = o+1;    // next token
    }

    // --- check request -------------------------------------------------------
    apr_hash_t* form = trell_tokenize_args( r->pool, r->args );
    if( o >= path_items->nelts ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "%s: path does not contain request.",
                       r->handler );
        return HTTP_BAD_REQUEST;
    }

    const char* request = APR_ARRAY_IDX( path_items, o, char*);


    // --- rpc.xml ---------------------------------------------------------
    if( apr_strnatcmp( request, "rpc.xml" ) == 0 ) {
        dispatch_info->m_request = TRELL_REQUEST_RPC_XML;

        if( dispatch_info->m_component == TRELL_COMPONENT_OPS ) {
            // component ops requires an action argument
            const char* action = apr_hash_get( form, "action", APR_HASH_KEY_STRING );
            if( action == NULL ) {

                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "%s: %s/%s: action required (args='%s'').",
                               r->handler, component, request, r->args );
                return HTTP_BAD_REQUEST;
            }
            if( apr_strnatcmp( action, "restart_master" ) == 0 ) {
                dispatch_info->m_mod_action = TRELL_MOD_ACTION_RESTART_MASTER;
            }
            else {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "%s: %s/%s: unknown action '%s'.",
                               r->handler, component, request, action );
                return HTTP_BAD_REQUEST;
            }
            return OK;
        }
        else {
            // components master and job use XML for actions.
            return OK;
        }
    }
    // --- getExposedModelUpdate.xml ---------------------------------------
    else if( apr_strnatcmp( request, "getExposedModelUpdate.xml" ) == 0 ) {
        dispatch_info->m_request = TRELL_REQUEST_POLICY_UPDATE_XML;
        /// FIXME: Should revision be optional?
        if( trell_hash_atoi( r, component, request, &dispatch_info->m_revision, form, "revision", 0 ) == 0 ) {
            return HTTP_BAD_REQUEST;
        }
        return OK;
    }
    // --- updateState.xml -------------------------------------------------
    else if( apr_strnatcmp( request, "updateState.xml" ) == 0 ) {
        dispatch_info->m_request = TRELL_REQUEST_STATE_UPDATE_XML;
        return OK;
    }
    // --- snapshot.png ----------------------------------------------------
    else if( strcmp( request, "snapshot.png" ) == 0 ) {
        dispatch_info->m_request = TRELL_REQUEST_PNG;
        dispatch_info->m_pixel_format = TRELL_PIXEL_FORMAT_BGR8;
        dispatch_info->m_base64 = 0;
        if( (trell_hash_strncpy( r, dispatch_info->m_key, form, "key", TRELL_KEYID_MAXLENGTH-1 ) == 0)
                || (trell_hash_atoi( r, component, request, &dispatch_info->m_width, form, "width", 1 ) == 0)
                || (trell_hash_atoi( r, component, request, &dispatch_info->m_height, form, "height", 1 ) == 0) )
        {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "%s: parsing %s failed.",
                           r->handler, request);
            return HTTP_BAD_REQUEST;
        }
    }
    // --- snapshot.txt----------------------------------------------------
    else if( strcmp( request, "snapshot.txt" ) == 0 ) {
        dispatch_info->m_request = TRELL_REQUEST_PNG;
        dispatch_info->m_pixel_format = TRELL_PIXEL_FORMAT_BGR8;
        dispatch_info->m_base64 = 1;
        if( (trell_hash_strncpy( r, dispatch_info->m_key, form, "key", TRELL_KEYID_MAXLENGTH-1 ) == 0 )
                || (trell_hash_atoi( r, component, request, &dispatch_info->m_width, form, "width", 1 ) == 0 )
                || (trell_hash_atoi( r, component, request, &dispatch_info->m_height, form, "height", 1 ) == 0 ) )
        {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "%s: parsing %s failed.",
                           r->handler, request );
            return HTTP_BAD_REQUEST;
        }
    }
    // --- snapshot_bundle.txt----------------------------------------------------
    else if( strcmp( request, "snapshot_bundle.txt" ) == 0 ) {
        dispatch_info->m_request = TRELL_REQUEST_PNG;
        dispatch_info->m_pixel_format = TRELL_PIXEL_FORMAT_BGR8_CUSTOM_DEPTH;
        dispatch_info->m_base64 = 1;
        if( (trell_hash_strncpy( r, dispatch_info->m_key, form, "key", TRELL_KEYID_MAXLENGTH-1 ) == 0 )
                || (trell_hash_atoi( r, component, request, &dispatch_info->m_width, form, "width", 1 ) == 0 )
                || (trell_hash_atoi( r, component, request, &dispatch_info->m_height, form, "height", 1 ) == 0 ) )
        {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "%s: parsing %s failed.",
                           r->handler, request );
            return HTTP_BAD_REQUEST;
        }
    }
    // --- getRenderList.xml -----------------------------------------------
    else if( strcmp( request, "getRenderList.xml" ) == 0 ) {
        dispatch_info->m_request = TRELL_REQUEST_GET_RENDERLIST;
        if( (trell_hash_strncpy( r, dispatch_info->m_key, form, "key", TRELL_KEYID_MAXLENGTH-1 ) == 0 )
                || (trell_hash_strncpy( r, dispatch_info->m_timestamp, form, "timestamp", TRELL_KEYID_MAXLENGTH-1 ) == 0) )
        {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "%s: parsing %s failed.",
                           r->handler, request );
            return HTTP_BAD_REQUEST;
        }
    }
    else if( strcmp ( request, "getScript.js" ) == 0 ) {
        dispatch_info->m_request = TRELL_REQUEST_GET_SCRIPT;
    }
    else {
        dispatch_info->m_request = TRELL_REQUEST_STATIC_FILE;

        // convert the remaining part of the request to a path

        apr_array_header_t* remaining = apr_array_make( r->pool, 0, sizeof(char*) );
        for( i=o; i<path_items->nelts; i++ ) {
            APR_ARRAY_PUSH( remaining, char*) = APR_ARRAY_IDX( path_items, i, char*);
        }
        dispatch_info->m_static_path = apr_array_pstrcat( r->pool, remaining, '/' );
        if( dispatch_info->m_static_path == NULL ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "%s: %s/%s path is NULL.",
                           r->handler, component, request );
            return HTTP_INTERNAL_SERVER_ERROR;
        }
    }
    return OK;
}
