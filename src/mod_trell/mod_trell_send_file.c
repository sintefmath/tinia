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
#include <apr_strings.h>
#include <apr_file_info.h>
#include "mod_trell.h"

int
trell_send_reply_static_file( trell_sconf_t*         sconf,
                              request_rec*           r,
                              trell_dispatch_info_t* dinfo )
{
    apr_status_t rv;
    char* path = NULL;

    if( dinfo->m_component == TRELL_COMPONENT_JOB ) {
        path = apr_pstrcat( r->pool, sconf->m_job_www_root, "/", dinfo->m_requestname, NULL );
    }
    if( path == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "Empty file." );
    }

    apr_finfo_t finfo;
    rv = apr_stat( &finfo, path, APR_FINFO_SIZE, r->pool);
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Failed to stat static file." );
        return HTTP_NOT_FOUND;
    }
    ap_set_content_length( r, finfo.size );

    // guess content type
    size_t l = strlen( path );
    if( (l>5) && (strcmp(&path[l-5], ".html") == 0 ) ) {
        ap_set_content_type( r, "text/html" );
    }
    else if( (l>3) && (strcmp(&path[l-3], ".js") == 0 ) ) {
        ap_set_content_type( r, "text/javascript" );
    }
    else if( (l>4) && (strcmp(&path[l-4], ".css" ) == 0 ) ) {
        ap_set_content_type( r, "text/css" );
    }
    else if( (l>4) && (strcmp(&path[l-4], ".gif" ) == 0 ) ) {
        ap_set_content_type( r, "image/gif" );
    }
    else if( (l>4) && (strcmp(&path[l-4], ".png" ) == 0 ) ) {
        ap_set_content_type( r, "image/png" );
    }

    apr_file_t* fd;
    rv = apr_file_open( &fd, path, APR_READ | APR_SENDFILE_ENABLED, APR_OS_DEFAULT, r->pool );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Failed to open static file." );
        return HTTP_NOT_FOUND;
    }

    apr_bucket_brigade* bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );
    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_file_create( fd, 0, finfo.size, r->pool, r->connection->bucket_alloc ) );
    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );


    rv = ap_pass_brigade( r->output_filters, bb );
    if( rv != APR_SUCCESS ) {
        apr_file_close( fd );
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Output error" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    else {
        apr_file_close( fd );
        return OK;
    }
}
