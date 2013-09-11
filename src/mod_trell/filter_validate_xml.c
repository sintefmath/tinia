#include <httpd.h>
#include <http_config.h>
#include <http_log.h>
#include <http_request.h>
#include <util_filter.h>
#include <apr_buckets.h>
#include <apr_strings.h>
#include "mod_trell.h"

typedef struct {
    apr_bucket_brigade*  m_bb;    
} tinia_xml_in_ctx_t;

apr_status_t
tinia_validate_xml_in_filter( ap_filter_t *f,
                              apr_bucket_brigade *bb,
                              ap_input_mode_t mode,
                              apr_read_type_e block,    // APR_BLOCK_READ or APR_NONBLOCK_READ
                              apr_off_t readbytes)
{
    // --- we only support mode == readbytes -----------------------------------
    if( mode != AP_MODE_READBYTES ) {
        ap_log_rerror( APLOG_MARK, APLOG_WARNING, 0, f->r,
                       "%s: only MODE_READMODE is supported.", f->frec->name );
        ap_remove_input_filter( f );
        return ap_get_brigade( f->next, bb, mode, block, readbytes );
    }

    
    //trell_sconf_t* sconf = ap_get_module_config( f->r->server->module_config, &trell_module );

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

    apr_status_t rv = ap_get_brigade( f->next, bb, mode, block, readbytes );

    apr_bucket* e = APR_BRIGADE_FIRST( bb );
    while( e != APR_BRIGADE_SENTINEL( bb ) ) {
        if( APR_BUCKET_IS_EOS(e) ) {
            // done, flatten brigade and do parsing.
        }
        e = APR_BUCKET_NEXT( e );
    }

    
    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, f->r, "tinia_validate_xml_in_filter invoked %s.",
                   f->r->path_info );
    
    return rv;


#if 0
    /* apr_bucket* b;
    const char* buf = NULL;
    apr_size_t bytes = 0;
    */
    // check if method is POST

    // check if content type is correct

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
#endif
    
/*    if( mode != AP_MODE_READBYTES ) {
        
    }
  */
    return ap_get_brigade( f->next, bb, mode, block, readbytes );
}

apr_status_t
tinia_validate_xml_out_filter( ap_filter_t *f,
                               apr_bucket_brigade *bb )
{
    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, f->r, "tinia_validate_xml_out_filter invoked.");
    return ap_pass_brigade( f->next, bb );
}
