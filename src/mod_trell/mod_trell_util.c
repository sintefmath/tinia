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
#include <apr_strings.h>
#include <util_filter.h>
#include <libxml/xmlreader.h>
#include <ctype.h>
#include "mod_trell.h"


struct TrellXMLReaderContext
{
    apr_bucket_brigade*  m_brigade;
    apr_array_header_t*  m_brigades;
    request_rec*         m_r;
};








static
int
trell_xml_reader_callback( void* context, char* buffer, int len )
{
    struct TrellXMLReaderContext* ctx = context;
    if( ctx->m_brigade == NULL ) {
        ctx->m_brigade = apr_brigade_create( ctx->m_r->pool,
                                             ctx->m_r->connection->bucket_alloc );
    }
    apr_status_t status = ap_get_brigade( ctx->m_r->input_filters,
                                          ctx->m_brigade,
                                          AP_MODE_READBYTES,
                                          APR_BLOCK_READ,
                                          len );
    if( status != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, status, ctx->m_r,
                       "mod_trell: Error getting XML content from input filters" );
        return -1;
    }
    apr_size_t read = len;
    apr_brigade_flatten( ctx->m_brigade, buffer, &read );
    if( ctx->m_brigades == NULL ) {
        apr_brigade_cleanup( ctx->m_brigade );
    }
    else {
        apr_bucket_brigade** n = apr_array_push( ctx->m_brigades );
        *n = ctx->m_brigade;
        ctx->m_brigade = NULL;
    }
    return read;
}

size_t
trell_flatten_brigades_into_mem( trell_sconf_t* sconf, request_rec*r,
                                 char* buffer, size_t buffer_size,
                                 apr_array_header_t* brigades )
{
    if( brigades == NULL ) {
        return 0u;
    }

    size_t offset = 0u;
    int i;
    for( i = 0; i<brigades->nelts; i++ ) {
        apr_size_t size = buffer_size-offset;
        apr_bucket_brigade* bb = ((apr_bucket_brigade**)(brigades->elts))[i];
        apr_brigade_flatten( bb, buffer + offset, &size );
        offset += size;
//        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r,
//                       "mod_trell: Flattened %d bytes", (int)size );
    }
    return offset;
}


/** Helper callback for logging libxml output to apache using server_rec. */
void
trell_xml_error_s_cb( void* ctx, const char* msg, ... )
{
    server_rec* s = (server_rec*)ctx;
    va_list args;
    char *fmsg;
    va_start( args, msg );
    fmsg = apr_pvsprintf( s->process->pool, msg, args );
    va_end( args );
    ap_log_perror( APLOG_MARK, APLOG_NOTICE, 0, s->process->pool,
                   "mod_trell: libxml2: %s", fmsg );
}

/** Helper callback for logging libxml output to apache using request_rec. */
static void
trell_xml_error_r_cb( void* ctx, const char* msg, ... )
{
    request_rec* r = (request_rec*)ctx;
    va_list args;
    char *fmsg;
    va_start( args, msg );
    fmsg = apr_pvsprintf( r->pool, msg, args );
    va_end( args );
    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r,
                   "mod_trell: libxml2: %s", fmsg );
}

void
trell_libxml_state_set_r( struct TrellLibXMLState* old, request_rec* r )
{
    old->m_original_error_context = xmlGenericErrorContext;
    old->m_original_error_func = xmlGenericError;
    xmlSetGenericErrorFunc( (void*)r, trell_xml_error_r_cb );
}

void
trell_libxml_state_restore( struct TrellLibXMLState* old )
{
    xmlSetGenericErrorFunc( old->m_original_error_context,
                            old->m_original_error_func );
}



int
trell_parse_xml( trell_sconf_t* sconf, request_rec* r, apr_array_header_t* brigades, xmlSchemaPtr schema )
{
    // check if method is POST
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

    int retval = OK;

    struct TrellLibXMLState libxml_state;
    trell_libxml_state_set_r( &libxml_state, r );


    xmlCharEncoding encoding = XML_CHAR_ENCODING_NONE;

    // create parser context
    struct TrellXMLReaderContext context;
    context.m_brigade = NULL;
    context.m_brigades = brigades;
    context.m_r = r;
    xmlParserInputBufferPtr input = xmlParserInputBufferCreateIO( trell_xml_reader_callback,
                                                                  NULL,
                                                                  &context,
                                                                  encoding );
    if( input == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: Failed to create xmlParserInputBuffer");
        retval = HTTP_INTERNAL_SERVER_ERROR;
    }
    else {
        xmlTextReaderPtr reader = xmlNewTextReader( input, "cloudviz.sintef.no/" );

        if( reader == NULL ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: Failed to create xmlTextReader" );
            retval = HTTP_INTERNAL_SERVER_ERROR;
        }
        else {
            // attach schema if specified
            xmlSchemaValidCtxtPtr schema_ctx = NULL;
            if( schema != NULL ) {
                schema_ctx = xmlSchemaNewValidCtxt( schema );
                if( schema_ctx != NULL ) {
                    if( xmlTextReaderSchemaValidateCtxt( reader, schema_ctx, 0 ) < 0 ) {
                        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                                       "mod_trell: Failure to attach schema to xml reader" );
                    }
                }
            }

            // parse document
            int ret = xmlTextReaderRead( reader );
            while( ret == 1 ) {
                ret = xmlTextReaderRead( reader );
            }
            if( ret != 0 ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "mod_trell: Failed to parse XML" );
                retval = HTTP_BAD_REQUEST;
            }

            if( xmlTextReaderIsValid( reader ) != 1 ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "mod_trell: Query XML is NOT valid" );
                retval = HTTP_BAD_REQUEST;
            }
            xmlFreeTextReader( reader );
            xmlFree( input );
            if( schema_ctx != NULL ) {
                xmlSchemaFreeValidCtxt( schema_ctx );
            }
        }
    }
    trell_libxml_state_restore( &libxml_state );

    if( context.m_brigade != NULL ) {
        apr_brigade_destroy( context.m_brigade );
    }
    return retval;
}


/** Parse argument list (foo=bar&)
  *
  * Note that the HTTP spec allows a key to appear multiple times, but we do not
  * support this. In case of multiple keys, the last instance is used.
  *
  */
apr_hash_t*
trell_parse_args_uniq_key( request_rec* r, char* args )
{

    if( args == NULL ) {
        return NULL;
    }
    apr_hash_t* form = apr_hash_make( r->pool );

    char* last;
    const char* delim = "&";
    char* pair = apr_strtok( args, delim, &last );
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
/*
        apr_array_header_t* values = apr_hash_get( form, pair, APR_HASH_KEY_STRING );
        if( values == NULL ) {
            values = apr_array_make( r->pool, 1, sizeof(const char*) );
            apr_hash_set( form, pair, APR_HASH_KEY_STRING, values );
        }
        char* ptr = apr_array_push( values );
        *ptr = apr_pstrdup( r->pool, eq );
*/

        // iterate forward
        pair = apr_strtok( NULL, delim, &last );
    }

    // Dump the parsed arguments to error log
    if( form ) {
        apr_hash_index_t* it = apr_hash_first( r->pool, form );
        while( it != NULL ) {
            const char* key;
            char* val;
            apr_hash_this( it, (const void**)&key, NULL, (void**)&val );
#if 0
            ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r,
                           "Key = '%s', value='%s'", key, val );
#endif
            it = apr_hash_next( it );
        }
    }

    return form;
}


int
trell_valid_jobid( trell_sconf_t* sconf, request_rec* r, const char* job )
{
    if( job == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: Job id is NULL" );
        return 0;
    }
    if( strcasecmp( sconf->m_master_id, job ) == 0 ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: Job id is master id" );
        return 0;
    }
    const char* p;
    for(p=job; *p != '\0'; p++ ) {
        if(!( isalnum( *p) || *p == '_' ) ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: Illegal character '%c' in job id", *p );
            return 0;
        }
    }
    return 1;
}
