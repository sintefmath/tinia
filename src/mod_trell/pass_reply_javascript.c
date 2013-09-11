#include <httpd.h>
#include <http_log.h>
#include <http_protocol.h>

#include "tinia/trell/trell.h"
#include "mod_trell.h"

int
trell_pass_reply_javascript( void* data,
                             unsigned char* buffer,
                             size_t offset,
                             size_t bytes,
                             int more )
{
    trell_callback_data_t* cbd = (trell_callback_data_t*)data;
    request_rec* r = cbd->m_r;

    trell_message_t* msg = (trell_message_t*)buffer;
    if( msg->m_type == TRELL_MESSAGE_SCRIPT ) {
        const char* payload = msg->m_script.m_script;
        size_t payload_size = msg->m_size;

        // --- trim trailing zeros, if present ---------------------------------        
        int n = 0;
        while( payload_size > 0 && payload[ payload_size-1 ] == '\0' ) {
            payload_size--;
            n++;
        }
        if( n > 0 ) {
            ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "Trimmed %d trailing zeros from javascript.", n );
        }

        // --- set http headers ------------------------------------------------        
        ap_set_content_type( r, "application/javascript" );
        ap_set_content_length( r, payload_size );

        char* datestring = apr_palloc( r->pool, APR_RFC822_DATE_LEN );
        apr_rfc822_date( datestring, apr_time_now() );
        apr_table_setn( r->headers_out, "Last-Modified", datestring );

        apr_table_setn( r->headers_out, "Cache-Control", "no-cache" );
        
        // --- ship contents ---------------------------------------------------
        apr_bucket_brigade* bb;
        apr_bucket* b;
    
        bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );
        b = apr_bucket_transient_create( payload, payload_size, bb->bucket_alloc );

        APR_BRIGADE_INSERT_TAIL( bb, b );
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );

        apr_status_t rv = ap_pass_brigade( r->output_filters, bb );
        if( rv != APR_SUCCESS ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Failed to pass brigade." );
            return -1;
        }
        else {
            return 0;
        }
    }
    else {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "Unexpected message type %d of size %d.",
                       (int)msg->m_type, (int)msg->m_size );
        return -1;
    }
}
