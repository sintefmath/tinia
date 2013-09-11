#include <httpd.h>
#include <http_config.h>
#include <http_log.h>
#include <http_request.h>
#include <util_filter.h>
#include <apr_buckets.h>

apr_status_t
tinia_validate_xml_in_filter( ap_filter_t *f,
                              apr_bucket_brigade *bb,
                              ap_input_mode_t mode,
                              apr_read_type_e block,
                              apr_off_t readbytes)
{
    if( !ap_is_initial_req(f->r) ) {
        ap_remove_input_filter(f);
        return ap_get_brigade( f->next, bb, mode, block, readbytes );
    }
    if( mode != AP_MODE_READBYTES ) {
        return ap_get_brigade( f->next, bb, mode, block, readbytes );
    }
    if( f->ctx != NULL ) {
        return ap_get_brigade( f->next, bb, mode, block, readbytes );
    }
    f->ctx = 0xDEADBEEF;
    
    // this one is invoked multiple times for a request, if ctx is null, it is
    // the first invocation.
    
    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, f->r, "tinia_validate_xml_in_filter invoked %s.",
                   f->r->path_info );

   /* apr_bucket* b;
    const char* buf = NULL;
    apr_size_t bytes = 0;
    */
    
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
