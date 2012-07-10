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
#include "mod_trell.h"


int
trell_decode_path_info( trell_dispatch_info_t* dispatch_info, request_rec *r )
{
    dispatch_info->m_component = TRELL_COMPONENT_NONE;
    dispatch_info->m_request = TRELL_REQUEST_NONE;
    dispatch_info->m_mod_action = TRELL_MOD_ACTION_NONE;
    dispatch_info->m_jobid[0] = '\0';
    dispatch_info->m_sessionid[0] = '\0';
    dispatch_info->m_requestname[0] = '\0';
    dispatch_info->m_key[0] ='\0';
    dispatch_info->m_timestamp[0] = '\0';
    dispatch_info->m_revision = 0;
    dispatch_info->m_base64 = 0;
    dispatch_info->m_width = 0;
    dispatch_info->m_height = 0;

    int i;
    char* p = r->path_info;

    if( p == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: path_info is NULL." );
        return HTTP_NOT_FOUND;
    }

    static const char* mod_prefix    = "/mod/";
    static const char* master_prefix = "/master/";
    static const char* job_prefix    = "/job/";


    // determine which component that should handle the request
    if( strncmp( p, mod_prefix, strlen(mod_prefix) ) == 0 ) {
        p += strlen(mod_prefix);
        dispatch_info->m_component = TRELL_COMPONENT_OPS;
    }
    else if( strncmp( p, master_prefix, strlen(master_prefix) ) == 0) {
        p += strlen(master_prefix);
        dispatch_info->m_component = TRELL_COMPONENT_MASTER;
    }
    else if( strncmp( p, job_prefix, strlen(job_prefix) ) == 0 ) {
        p += strlen(job_prefix);
        dispatch_info->m_component = TRELL_COMPONENT_JOB;
    }
    else {
        return HTTP_NOT_FOUND;
    }

    // If job path, determine job id
    if( dispatch_info->m_component == TRELL_COMPONENT_JOB ) {
        for( i=0; i<TRELL_JOBID_MAXLENGTH-1; i++) {
            if( (i>0) && (*p == '/') ) {
                break;      // found separator
            }
            else if( !(isalnum(*p) || (*p=='_' ) ) ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: Illegal char in jobid." );
                return HTTP_NOT_FOUND;
            }
            dispatch_info->m_jobid[i] = *p++;
        }
        dispatch_info->m_jobid[i] = '\0';
        if( *p++ != '/' ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: Jbid too long." );
            return HTTP_NOT_FOUND;
        }
    }

    // If job path, determine session id
    if( dispatch_info->m_component == TRELL_COMPONENT_JOB ) {
        for( i=0; i<TRELL_SESSIONID_MAXLENGTH-1; i++ ) {
            if( (i>0) && (*p == '/') ) {
                break;      // found separator
            }
            else if( !(isalnum(*p) || (*p=='_' ) ) ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: Illegal char in sessionid." );
                return HTTP_NOT_FOUND;
            }
            dispatch_info->m_sessionid[i] = *p++;
        }
        dispatch_info->m_sessionid[i] = '\0';
        if( *p++ != '/' ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: Sessionid too long." );
            return HTTP_NOT_FOUND;
        }
    }



    // Parse arguments. Note that the HTTP protocol allows multiple entries
    // with the same keys, but we disallow it.
    apr_hash_t* form = apr_hash_make( r->pool );
    if( r->args != NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: args=%s.", r->args );
        char* last;
        const char* delim = "&";
        char* pair = apr_strtok( r->args, delim, &last );
        while( pair != NULL ) {
            char* eq = NULL;
            char* t;
            for( t = pair; *t; ++t ) {
                if( *t == '+' ) {
                    *t = ' ';
                }
                else if( eq == NULL && *t == '=' ) {
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
            apr_hash_set( form, pair, APR_HASH_KEY_STRING, apr_pstrdup( r->pool, eq ) );

            // iterate forward
            pair = apr_strtok( NULL, delim, &last );
        }
    }
    else {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: no args" );
    }


    int want_revision     = 0;
    int require_key       = 0;
    int require_width     = 0;
    int require_height    = 0;
    int require_action    = 0;
    int require_timestamp = 0;

    // Determine request
    for( i=0; i<TRELL_SESSIONID_MAXLENGTH-1; i++ ) {
        if( (i>0) && ( (*p=='\0' ) || (*p=='?') ) ) {
            break;      // found separator
        }
        else if( !(isalnum(*p) || (*p=='_' ) || (*p=='.' ) || (*p=='/' ) || (*p=='-' ) ) ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: Illegal char in request." );
            return HTTP_NOT_FOUND;
        }
        dispatch_info->m_requestname[i] = *p++;
    }
    dispatch_info->m_requestname[i] = '\0';
    if( (*p != '\0' ) && (*p != '?' ) ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: Request too long." );
        return HTTP_NOT_FOUND; // Request too long
    }


    // determine dispatch path
    if( strcmp( dispatch_info->m_requestname, "rpc.xml" ) == 0 ) {
        require_action = 1;
        dispatch_info->m_request = TRELL_REQUEST_RPC_XML;
    }
    else if( strcmp( dispatch_info->m_requestname, "getExposedModelUpdate.xml" ) == 0 ) {
        want_revision = 1;
        dispatch_info->m_request = TRELL_REQUEST_POLICY_UPDATE_XML;
    }
    else if( strcmp( dispatch_info->m_requestname, "updateState.xml" ) == 0 ) {
        dispatch_info->m_request = TRELL_REQUEST_STATE_UPDATE_XML;
    }
    else if( strcmp( dispatch_info->m_requestname, "snapshot.png" ) == 0 ) {
        require_key    = 1;
        require_width  = 1;
        require_height = 1;
        dispatch_info->m_request = TRELL_REQUEST_PNG;
    }
    else if( strcmp( dispatch_info->m_requestname, "snapshot.txt" ) == 0 ) {
        require_key    = 1;
        require_width  = 1;
        require_height = 1;
        dispatch_info->m_request = TRELL_REQUEST_PNG;
        dispatch_info->m_base64 = 1;
    }
    else if( strcmp( dispatch_info->m_requestname, "getRenderList.xml" ) == 0 ) {
        require_key       = 1;
        require_timestamp = 1;
        dispatch_info->m_request = TRELL_REQUEST_GET_RENDERLIST;
    }
    else if( strcmp ( dispatch_info->m_requestname, "getScript.js" ) == 0 ) {
        dispatch_info->m_request = TRELL_REQUEST_GET_SCRIPT;
    }
    else {
        dispatch_info->m_request = TRELL_REQUEST_STATIC_FILE;
    }


    // process arguments
    if( want_revision != 0 ) {
        const char* p = apr_hash_get( form, "revision", APR_HASH_KEY_STRING );
        if( p != NULL ) {
            dispatch_info->m_revision = atoi( p );
        }
    }
    if( require_width  != 0 ) {
        const char* p = apr_hash_get( form, "width", APR_HASH_KEY_STRING );
        if( p == NULL ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: width is required" );
            return HTTP_BAD_REQUEST;
        }
        dispatch_info->m_width = atoi( p );

    }
    if( require_height  != 0 ) {
        const char* p = apr_hash_get( form, "height", APR_HASH_KEY_STRING );
        if( p == NULL ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: height is required" );
            return HTTP_BAD_REQUEST;
        }
        dispatch_info->m_height = atoi( p );
    }
    if( require_action  != 0 ) {
        const char* p = apr_hash_get( form, "action", APR_HASH_KEY_STRING );
        if( p != NULL ) {
            if( strcmp( p, "restart_master" ) == 0 ) {
                dispatch_info->m_mod_action = TRELL_MOD_ACTION_RESTART_MASTER;
            }
            else {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: RPC: unknown action '%s'", p );
                return HTTP_BAD_REQUEST;
            }
        }
    }
    if( require_key  != 0 ) {
        const char* p = apr_hash_get( form, "key", APR_HASH_KEY_STRING );
        if( p == NULL ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: key is required" );
            return HTTP_BAD_REQUEST;
        }
        else if( strlen( p ) > TRELL_KEYID_MAXLENGTH-1 ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: key too long" );
            return HTTP_BAD_REQUEST;
        }
        strcpy( dispatch_info->m_key, p );
    }
    if( require_timestamp  != 0 ) {
        const char* p = apr_hash_get( form, "timestamp", APR_HASH_KEY_STRING );
        if( p == NULL ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: Timestamp is required" );
            return HTTP_BAD_REQUEST;
        }
        else if( strlen( p ) > TRELL_TIMESTAMP_MAXLENGTH-1 ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: Timestamp too long" );
            return HTTP_BAD_REQUEST;
        }
        strcpy( dispatch_info->m_timestamp, p );
    }



    if( *p != '\0' ) {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "mod_trell: ignored '%s' of request.", p );
    }
    return OK;
}
