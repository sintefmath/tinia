#include <httpd.h>
#include <http_config.h>
#include <http_log.h>
#include <http_request.h>
#include <util_filter.h>
#include <apr_buckets.h>
#include <apr_strings.h>

//#include <libxml/parser.h>  // xmlReadMemory
#include <libxml/xmlreader.h>
#include "mod_trell.h"

typedef struct {
    apr_bucket_brigade*  m_bb;    
} tinia_xml_in_ctx_t;

/** Callback to be used with xmlSetGenericErrorFunc. */
static
void
tinia_filter_libxml_generic_error_func( void* arg, const char* msg, ... )
{
    ap_filter_t* f = (ap_filter_t*)arg;
    va_list args;
    char *fmsg;
    va_start( args, msg );
    fmsg = apr_pvsprintf( f->r->pool, msg, args );
    va_end( args );
    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, f->r,
                   "%s-gef: libxml2: %s", f->frec->name, fmsg );
}

static
void
tinia_filter_libxml_structured_error_func( void* arg,
                                           xmlErrorPtr error )
{
    ap_filter_t* f = (ap_filter_t*)arg;
    ap_log_rerror( APLOG_MARK, APLOG_WARNING, 0, f->r,
                   "%s-sef: libxml2: structured error.", f->frec->name );
}

static
void
tinia_filter_libxml_error_func( void* arg,
                             const char* msg,
                             xmlParserSeverities severity,
                             xmlTextReaderLocatorPtr locator )
{
    ap_filter_t* f = (ap_filter_t*)arg;
    ap_log_rerror( APLOG_MARK, APLOG_WARNING, 0, f->r,
                   "%s-ef: libxml2: %s.", f->frec->name, msg );
}

static
apr_status_t
tinia_validate_xml_helper( ap_filter_t *f,
                           apr_bucket_brigade *bb,
                           xmlSchemaPtr schema,
                           const char* url )
{
    
    // --- flatten brigade -----------------------------------------------------
    char* doc = NULL;
    apr_size_t len = 0;
    int rv = apr_brigade_pflatten( bb, &doc, &len, f->r->pool );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, f->r,
                       "%s: apr_brigade_pflatten failed.", f->frec->name );
        return rv;
    }
    rv = APR_EGENERAL; // assume error until success.
    if( (doc == NULL) || (len==0) ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, f->r,
                       "%s: no data.", f->frec->name );
        return rv;
    }
    
    // Unfortunately, it seems that only this one gets the error messages,
    // I would prefer not to mess with global state.
    void*                old_error_context = xmlGenericErrorContext;
    xmlGenericErrorFunc  old_error_func = xmlGenericError;
    xmlSetGenericErrorFunc( f, tinia_filter_libxml_generic_error_func );
    
    // -- Create and attach input, reader and validator ------------------------
    xmlParserInputBufferPtr input = xmlParserInputBufferCreateStatic( doc,
                                                                      len,
                                                                      XML_CHAR_ENCODING_NONE );
    if( input == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, f->r,
                       "%s: Failed to create libxml2 input buffer.", f->frec->name );
    }
    else {
        xmlTextReaderPtr reader = xmlNewTextReader( input, url );
        if( reader == NULL ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, f->r,
                           "%s: Failed to create libxml2 reader.", f->frec->name );
        }
        else {
            // Not sure if this one is used.
            xmlTextReaderSetErrorHandler( reader, tinia_filter_libxml_error_func, f );
            // Not sure if this one is used either.
            xmlTextReaderSetStructuredErrorHandler( reader, tinia_filter_libxml_structured_error_func, f );

            xmlSchemaValidCtxtPtr validator = xmlSchemaNewValidCtxt( schema );
            if( validator == NULL ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, f->r,
                               "%s: Failed to create libxml2 schema validator.", f->frec->name );
            }
            else {
                xmlTextReaderSchemaValidateCtxt( reader, validator, 0 );
    
                // --- parse xml and check if it is valid ----------------------        
                int ret = xmlTextReaderRead( reader );
                while( ret == 1 ) {
                    ret = xmlTextReaderRead( reader );
                }
                if( ret != 0 ) {
                    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, f->r,
                                   "%s: Failed to parse XML: %s", f->frec->name, f->r->path_info );
                }
                else {
                    if( xmlTextReaderIsValid( reader ) != 1 ) {
                        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, f->r,
                                       "%s: Failed to validate XML: %s", f->frec->name, f->r->path_info );
                    }
                    else {
                        //ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, f->r,
                        //               "%s: valid input xml: %s", f->frec->name, f->r->path_info );
                        rv = APR_SUCCESS; // xml is parsed and validated!
                    }
                }
                xmlTextReaderSchemaValidateCtxt( reader, NULL, 0 );
                xmlSchemaFreeValidCtxt( validator );
            }
            xmlFreeTextReader( reader );
        }
        xmlFreeParserInputBuffer( input );
    }
    xmlSetGenericErrorFunc( old_error_context, old_error_func );
    return rv;
}


apr_status_t
tinia_validate_xml_in_filter( ap_filter_t *f,
                              apr_bucket_brigade *bb,
                              ap_input_mode_t mode,
                              apr_read_type_e block,    // APR_BLOCK_READ or APR_NONBLOCK_READ
                              apr_off_t readbytes)
{
    // --- get the config for this request -------------------------------------
    req_cfg_t* req_cfg = ap_get_module_config( f->r->request_config, tinia_get_module() );
    if( req_cfg == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_WARNING, 0, f->r,
                       "%s: missing request config.", f->frec->name );
        ap_remove_input_filter( f );
        return ap_get_brigade( f->next, bb, mode, block, readbytes );
    }
    
    // --- we need a schema if we're going to validate -------------------------
    if( req_cfg->m_schema == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_WARNING, 0, f->r,
                       "%s: missing schema.", f->frec->name );
        
        ap_remove_input_filter( f );
        return ap_get_brigade( f->next, bb, mode, block, readbytes );
    }
    
    // --- we only support mode == readbytes -----------------------------------
    if( mode != AP_MODE_READBYTES ) {
        ap_log_rerror( APLOG_MARK, APLOG_WARNING, 0, f->r,
                       "%s: only MODE_READMODE is supported.", f->frec->name );
        ap_remove_input_filter( f );
        return ap_get_brigade( f->next, bb, mode, block, readbytes );
    }

    tinia_xml_in_ctx_t* ctx = f->ctx;
    // --- handle first run of filter for this connection ----------------------
    if( ctx == NULL ) {

        // --- only work on main requests --------------------------------------
        if( !ap_is_initial_req(f->r) ) {
            ap_remove_input_filter(f);
            return ap_get_brigade( f->next, bb, mode, block, readbytes );
        }
        
        // --- get content type ------------------------------------------------
        const char* content_type = apr_pstrdup( f->r->pool,
                                                apr_table_get( f->r->headers_in,
                                                               "Content-Type" ) );
        if( content_type == NULL ) {
            ap_log_rerror(  APLOG_MARK, APLOG_ERR, 0, f->r,
                            "%s: No content type.", f->frec->name);
            ap_remove_input_filter(f);
            return ap_get_brigade( f->next, bb, mode, block, readbytes );
        }
        
        // --- handle 'Content-Type: text/xml; encoding=foo' -------------------
        char* semicolon = strchr( content_type, ';' );
        if( semicolon != NULL ) {
            *semicolon = '\0'; // Chop line at semicolon.
        }
        
        // --- check content type ----------------------------------------------
        if( ! ( ( strcasecmp( "application/xml", content_type) == 0 ) ||
                ( strcasecmp( "text/xml", content_type ) == 0 ) ) )
        {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, f->r,
                           "%s: Content-type '%s' is not recognized as XML.",
                           f->frec->name, content_type );
            ap_remove_input_filter(f);
            return ap_get_brigade( f->next, bb, mode, block, readbytes );
        }
        
        // --- create filter context -------------------------------------------
        ctx = f->ctx = apr_palloc( f->r->pool, sizeof(tinia_xml_in_ctx_t) );
        ctx->m_bb = apr_brigade_create( f->r->pool, f->r->connection->bucket_alloc);
    }

    // --- pull data from previous filters and populate my own brigade ---------
    apr_status_t rv = ap_get_brigade( f->next, bb, mode, block, readbytes );
    apr_bucket* e = APR_BRIGADE_FIRST( bb );
    while( e != APR_BRIGADE_SENTINEL( bb ) ) {
        // copy bucket into our own private brigade
        apr_bucket* c = NULL;
        apr_bucket_copy( e, &c );
        APR_BRIGADE_INSERT_TAIL( ctx->m_bb, c );

        // we have reached the end of the input
        if( APR_BUCKET_IS_EOS(e) ) {
            // validate xml
            rv = tinia_validate_xml_helper( f,
                                            ctx->m_bb,
                                            req_cfg->m_schema, 
                                            "tinia.sintef.no" );
            // and clean up
            rv = apr_brigade_destroy( ctx->m_bb );
            if( rv != APR_SUCCESS ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, f->r,
                               "%s: apr_brigade_destroy failed.", f->frec->name );
            }
            ctx->m_bb = NULL;
            ap_remove_input_filter(f);  // remove ourselves for roboustness.
            break;
        }
        e = APR_BUCKET_NEXT( e );
    }
    return rv;
}

apr_status_t
tinia_validate_xml_out_filter( ap_filter_t *f,
                               apr_bucket_brigade *bb )
{
    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, f->r, "tinia_validate_xml_out_filter invoked.");
    return ap_pass_brigade( f->next, bb );
}
