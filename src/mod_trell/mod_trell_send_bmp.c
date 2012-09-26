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
trell_send_bmp( request_rec* r,
                trell_dispatch_info_t*  dispatch_info,
                struct messenger* msgr )
{
//    return HTTP_INTERNAL_SERVER_ERROR;

    apr_time_t a = apr_time_now();

    struct trell_message* msg = msgr->m_shmem_ptr;

    size_t w = msg->m_image.m_width;
    size_t h = msg->m_image.m_height;
    size_t size = 14 + 12 + 3*w*h;

    // --- create header ---
    unsigned char* bmp = apr_palloc( r->pool, size );

//    char header[ 14+12 ];

    // bmp file header, 14 bytes
    bmp[ 0] = 'B';
    bmp[ 1] = 'M';
    bmp[ 2] = (size&0xffu);
    bmp[ 3] = ((size>>8)&0xffu);
    bmp[ 4] = ((size>>16)&0xffu);
    bmp[ 5] = ((size>>24)&0xffu);
    bmp[ 6] = 0;
    bmp[ 7] = 0;
    bmp[ 8] = 0;
    bmp[ 9] = 0;
    bmp[10] = ((14+12))&0xffu;
    bmp[11] = ((14+12)>>8)&0xffu;
    bmp[12] = ((14+12)>>16)&0xffu;
    bmp[13] = ((14+12)>>24)&0xffu;

    // OS/2 V1 DIB header, 12 bytes
    bmp[14 +  0] = ((12)>>0)&0xffu;
    bmp[14 +  1] = ((12)>>8)&0xffu;
    bmp[14 +  2] = ((12)>>16)&0xffu;
    bmp[14 +  3] = ((12)>>24)&0xffu;
    bmp[14 +  4] = ((w)>>0)&0xffu;
    bmp[14 +  5] = ((w)>>8)&0xffu;
    bmp[14 +  6] = ((h)>>0)&0xffu;
    bmp[14 +  7] = ((h)>>8)&0xffu;
    bmp[14 +  8] = ((1)>>0)&0xffu; // color planes (=1)
    bmp[14 +  9] = ((1)>>8)&0xffu;
    bmp[14 + 10] = ((24)>>0)&0xffu; // bpp (=24)
    bmp[14 + 11] = ((24)>>8)&0xffu;


    // todo: release

    char* datestring = apr_palloc( r->pool, APR_RFC822_DATE_LEN );
    apr_rfc822_date( datestring, apr_time_now() );
    apr_table_setn( r->headers_out, "Last-Modified", datestring );
    apr_table_setn( r->headers_out, "Cache-Control", "no-cache" );
    struct apr_bucket_brigade* bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );

    if( dispatch_info->m_base64 == 0 ) {
        ap_set_content_type( r, "image/x-bmp" );
        ap_set_content_length( r, size );
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( bmp, 14+12, bb->bucket_alloc ) );
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( msg->m_image.m_data, 3*w*h, bb->bucket_alloc ) );
    }
    else {
        apr_table_setn( r->headers_out, "Content-Type", "text/plain" );
        ap_set_content_type( r, "text/plain" );
        memcpy( bmp+14+12, msg->m_image.m_data, 3*w*h );
        char* base64 = apr_palloc( r->pool, apr_base64_encode_len( size ) );
        int base64_size = apr_base64_encode( base64, bmp, size );
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_immortal_create( base64, base64_size, bb->bucket_alloc ) );
    }
    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );



    apr_status_t rv = ap_pass_brigade( r->output_filters, bb );

    apr_time_t q = apr_time_now();

    float foo = ((float)(q-a)*(1000.0f/APR_USEC_PER_SEC));
    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "mod_trell: %d: sent bmp, used %f ms", getpid(), foo );


    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Output error" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    else {
        return OK;
    }

}
