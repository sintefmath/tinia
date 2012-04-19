#include <httpd.h>
#include <http_log.h>
#include <http_protocol.h>
#include <apr_strings.h>
#include <apr_file_info.h>
#include "mod_trell.h"

int
trell_send_reply_bmp( request_rec* r, struct messenger* msgr )
{
    return HTTP_INTERNAL_SERVER_ERROR;
/*
    struct trell_message* msg = msgr->m_shmem_ptr;
    if( msg->m_type != TRELL_MESSAGE_IMAGE_BGR ) {
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    size_t w = msg->m_image_payload.m_width;
    size_t h = msg->m_image_payload.m_height;
    size_t size = 14 + 12 + 3*w*h;

    // --- create header ---
    char header[ 14+12 ];

    // bmp file header, 14 bytes
    header[ 0] = 'B';
    header[ 1] = 'M';
    header[ 2] = (size&0xffu);
    header[ 3] = ((size>>8)&0xffu);
    header[ 4] = ((size>>16)&0xffu);
    header[ 5] = ((size>>24)&0xffu);
    header[ 6] = 0;
    header[ 7] = 0;
    header[ 8] = 0;
    header[ 9] = 0;
    header[10] = ((14+12))&0xffu;
    header[11] = ((14+12)>>8)&0xffu;
    header[12] = ((14+12)>>16)&0xffu;
    header[13] = ((14+12)>>24)&0xffu;

    // OS/2 V1 DIB header, 12 bytes
    header[14 +  0] = ((12)>>0)&0xffu;
    header[14 +  1] = ((12)>>8)&0xffu;
    header[14 +  2] = ((12)>>16)&0xffu;
    header[14 +  3] = ((12)>>24)&0xffu;
    header[14 +  4] = ((w)>>0)&0xffu;
    header[14 +  5] = ((w)>>8)&0xffu;
    header[14 +  6] = ((h)>>0)&0xffu;
    header[14 +  7] = ((h)>>8)&0xffu;
    header[14 +  8] = ((1)>>0)&0xffu; // color planes (=1)
    header[14 +  9] = ((1)>>8)&0xffu;
    header[14 + 10] = ((24)>>0)&0xffu; // bpp (=24)
    header[14 + 11] = ((24)>>8)&0xffu;

    char* datestring = apr_palloc( r->pool, APR_RFC822_DATE_LEN );
    apr_rfc822_date( datestring, apr_time_now() );

    ap_set_content_type( r, "image/x-bmp" );
    ap_set_content_length( r, size );
    apr_table_setn( r->headers_out, "Last-Modified", datestring );

    struct apr_bucket_brigade* bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );
    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( header, 14+12, bb->bucket_alloc ) );
    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( msg->m_image_payload.m_data, 3*w*h, bb->bucket_alloc ) );
    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );

    apr_status_t rv = ap_pass_brigade( r->output_filters, bb );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Output error" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    else {
        return OK;
    }
*/
}
