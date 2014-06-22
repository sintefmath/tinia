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

#include <assert.h>
#include <zlib.h>
#include <httpd.h>
#include <http_log.h>
#include <http_protocol.h>
#include <apr_strings.h>
#include <apr_file_info.h>
#include <apr_base64.h>
#include "tinia/trell/trell.h"
#include "mod_trell.h"

unsigned long int
trell_png_crc( const unsigned long* crc_table, unsigned char* p, size_t length )
{
    size_t i;
    unsigned long int crc = 0xffffffffl;
    for(i=0; i<length; i++) {
        unsigned int ix = ( p[i]^crc ) & 0xff;
        crc = crc_table[ix]^((crc>>8));
    }
    return ~crc;
}


int
trell_send_png( trell_sconf_t*          sconf,
                request_rec*            r,
                trell_dispatch_info_t*  dispatch_info,
                enum TrellPixelFormat   format,
                const int               width,
                const int               height,
                const char*             payload,
                const size_t            payload_size )
{
    dispatch_info->m_png_entry = apr_time_now();
    int i, j;

    // Todo: Move this into init code. Some care must be taken to make sure
    // it is reentrant.
    unsigned long crc_table[ 256 ];
    for( j=0; j<256; j++) {
        unsigned long int c = j;
        for( i=0; i<8; i++) {
            if( c & 0x1 ) {
                c = 0xedb88320ul ^ (c>>1);
            }
            else {
                c = c>>1;
            }
        }
        crc_table[j] = c;
    }

    // Apply a PNG prediction filter, and do it upside-down effectively
    // flipping the image.

    char* filtered = apr_palloc( r->pool, (3*width+1)*height );
    dispatch_info->m_png_filter_entry = apr_time_now();
    for( j=0; j<height; j++) {
        filtered[ (3*width+1)*j + 0 ] = 0;
        for(i=0; i<width; i++) {
            filtered[ (3*width+1)*j + 1 + 3*i + 0 ] = payload[ 3*width*(height-j-1) + 3*i + 2 ];
            filtered[ (3*width+1)*j + 1 + 3*i + 1 ] = payload[ 3*width*(height-j-1) + 3*i + 1 ];
            filtered[ (3*width+1)*j + 1 + 3*i + 2 ] = payload[ 3*width*(height-j-1) + 3*i + 0 ];
        }
    }
    dispatch_info->m_png_filter_exit = apr_time_now();


    uLong bound = compressBound( (3*width+1)*height );
    unsigned char* png = apr_palloc( r->pool, bound + 8 + 25 + 12 + 12 + 12 );
    unsigned char* p = png;

    // PNG signature, 8 bytes
    *p++ = 137;
    *p++ = 80;
    *p++ = 78;
    *p++ = 71;
    *p++ = 13;
    *p++ = 10;
    *p++ = 26;
    *p++ = 10;

    // IHDR chunk, 13 + 12 (length, type, crc) = 25 bytes
    *p++ = ((13)>>24)&0xffu;   // chunk length
    *p++ = ((13)>>16)&0xffu;
    *p++ = ((13)>>8)&0xffu;
    *p++ = ((13)>>0)&0xffu;
    *p++ = 'I';                // chunk type
    *p++ = 'H';
    *p++ = 'D';
    *p++ = 'R';
    *p++ = ((width)>>24)&0xffu;    // image width
    *p++ = ((width)>>16)&0xffu;
    *p++ = ((width)>>8)&0xffu;
    *p++ = ((width)>>0)&0xffu;
    *p++ = ((height)>>24)&0xffu;    // image height
    *p++ = ((height)>>16)&0xffu;
    *p++ = ((height)>>8)&0xffu;
    *p++ = ((height)>>0)&0xffu;
    *p++ = 8;                  // 8 bits per channel
    *p++ = 2;                  // RGB triple
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;                  // image not interlaced
    unsigned int crc = trell_png_crc( crc_table, p-13-4, 13+4 );
    *p++ = ((crc)>>24)&0xffu;    // image width
    *p++ = ((crc)>>16)&0xffu;
    *p++ = ((crc)>>8)&0xffu;
    *p++ = ((crc)>>0)&0xffu;

    // IDAT chunk, 12 + payload bytes in total.

    dispatch_info->m_png_compress_entry = apr_time_now();
    int c = compress( (Bytef*)(p+8), &bound, (Bytef*)filtered, (3*width+1)*height );
    dispatch_info->m_png_compress_exit = apr_time_now();
    if( c == Z_MEM_ERROR ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "Z_MEM_ERROR" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    else if( c == Z_BUF_ERROR ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "Z_BUF_ERROR" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    *p++ = ((bound)>>24)&0xffu;    // compressed image data size
    *p++ = ((bound)>>16)&0xffu;
    *p++ = ((bound)>>8)&0xffu;
    *p++ = ((bound)>>0)&0xffu;
    *p++ = 'I';                // chunk type
    *p++ = 'D';
    *p++ = 'A';
    *p++ = 'T';

    p += bound;                // skip forward
    crc = trell_png_crc( crc_table, p-bound-4, bound+4 );
    *p++ = ((crc)>>24)&0xffu;    // CRC
    *p++ = ((crc)>>16)&0xffu;
    *p++ = ((crc)>>8)&0xffu;
    *p++ = ((crc)>>0)&0xffu;

    // IEND chunk
    *p++ = 0;       // zero payload
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 'I';     // ID
    *p++ = 'E';
    *p++ = 'N';
    *p++ = 'D';
    *p++ = 174;     // CRC
    *p++ = 66;
    *p++ = 96;
    *p++ = 130;
  
    
    char* datestring = apr_palloc( r->pool, APR_RFC822_DATE_LEN );
    apr_rfc822_date( datestring, apr_time_now() );
    apr_table_setn( r->headers_out, "Last-Modified", datestring );
    apr_table_setn( r->headers_out, "Cache-Control", "no-cache" );

    struct apr_bucket_brigade* bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );
    if( dispatch_info->m_base64 == 0 ) {
        // Send as plain png image
        ap_set_content_type( r, "image/png" );
        ap_set_content_length( r, p-png );
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_immortal_create( (char*)png, p-png, bb->bucket_alloc ) );
    }
    else {
        // Encode png as base64 and send as string
        apr_table_setn( r->headers_out, "Content-Type", "text/plain" );
        ap_set_content_type( r, "text/plain" );
        char* base64 = apr_palloc( r->pool, apr_base64_encode_len( p-png ) );
        int base64_size = apr_base64_encode( base64, (char*)png, p-png );
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_immortal_create( base64, base64_size, bb->bucket_alloc ) );
    }
    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );

    apr_status_t rv = ap_pass_brigade( r->output_filters, bb );
    dispatch_info->m_png_exit = apr_time_now();

    
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Output error" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    else {
        return OK;
    }
}



int
trell_png_encode( trell_sconf_t*          sconf,
                  request_rec*            r,
                  trell_dispatch_info_t*  dispatch_info,
                  const int               width,
                  const int               height,
                  const char*             payload,
                  unsigned char **        png_ptr,
                  unsigned char **        p_ptr )
{

    dispatch_info->m_png_entry = apr_time_now();
    int i, j;

    // Todo: Move this into init code. Some care must be taken to make sure
    // it is reentrant.
    unsigned long crc_table[ 256 ];
    for( j=0; j<256; j++) {
        unsigned long int c = j;
        for( i=0; i<8; i++) {
            if( c & 0x1 ) {
                c = 0xedb88320ul ^ (c>>1);
            }
            else {
                c = c>>1;
            }
        }
        crc_table[j] = c;
    }

    // Apply a PNG prediction filter, and do it upside-down effectively
    // flipping the image.

    char* filtered = apr_palloc( r->pool, (3*width+1)*height );
    dispatch_info->m_png_filter_entry = apr_time_now();
    for( j=0; j<height; j++) {
        filtered[ (3*width+1)*j + 0 ] = 0;
        for(i=0; i<width; i++) {
            filtered[ (3*width+1)*j + 1 + 3*i + 0 ] = payload[ 3*width*(height-j-1) + 3*i + 2 ];
            filtered[ (3*width+1)*j + 1 + 3*i + 1 ] = payload[ 3*width*(height-j-1) + 3*i + 1 ];
            filtered[ (3*width+1)*j + 1 + 3*i + 2 ] = payload[ 3*width*(height-j-1) + 3*i + 0 ];
        }
    }
    dispatch_info->m_png_filter_exit = apr_time_now();

    uLong bound = compressBound( (3*width+1)*height );
    unsigned char* png = apr_palloc( r->pool, bound + 8 + 25 + 12 + 12 + 12 );
    unsigned char* p = png;

    // PNG signature, 8 bytes
    *p++ = 137;
    *p++ = 80;
    *p++ = 78;
    *p++ = 71;
    *p++ = 13;
    *p++ = 10;
    *p++ = 26;
    *p++ = 10;

    // IHDR chunk, 13 + 12 (length, type, crc) = 25 bytes
    *p++ = ((13)>>24)&0xffu;   // chunk length
    *p++ = ((13)>>16)&0xffu;
    *p++ = ((13)>>8)&0xffu;
    *p++ = ((13)>>0)&0xffu;
    *p++ = 'I';                // chunk type
    *p++ = 'H';
    *p++ = 'D';
    *p++ = 'R';
    *p++ = ((width)>>24)&0xffu;    // image width
    *p++ = ((width)>>16)&0xffu;
    *p++ = ((width)>>8)&0xffu;
    *p++ = ((width)>>0)&0xffu;
    *p++ = ((height)>>24)&0xffu;    // image height
    *p++ = ((height)>>16)&0xffu;
    *p++ = ((height)>>8)&0xffu;
    *p++ = ((height)>>0)&0xffu;
    *p++ = 8;                  // 8 bits per channel
    *p++ = 2;                  // RGB triple
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;                  // image not interlaced
    unsigned int crc = trell_png_crc( crc_table, p-13-4, 13+4 );
    *p++ = ((crc)>>24)&0xffu;    // image width
    *p++ = ((crc)>>16)&0xffu;
    *p++ = ((crc)>>8)&0xffu;
    *p++ = ((crc)>>0)&0xffu;

    // IDAT chunk, 12 + payload bytes in total.

    dispatch_info->m_png_compress_entry = apr_time_now();
    int c = compress( (Bytef*)(p+8), &bound, (Bytef*)filtered, (3*width+1)*height );
    dispatch_info->m_png_compress_exit = apr_time_now();
    if( c == Z_MEM_ERROR ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "Z_MEM_ERROR" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    else if( c == Z_BUF_ERROR ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "Z_BUF_ERROR" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    *p++ = ((bound)>>24)&0xffu;    // compressed image data size
    *p++ = ((bound)>>16)&0xffu;
    *p++ = ((bound)>>8)&0xffu;
    *p++ = ((bound)>>0)&0xffu;
    *p++ = 'I';                // chunk type
    *p++ = 'D';
    *p++ = 'A';
    *p++ = 'T';

    p += bound;                // skip forward
    crc = trell_png_crc( crc_table, p-bound-4, bound+4 );
    *p++ = ((crc)>>24)&0xffu;    // CRC
    *p++ = ((crc)>>16)&0xffu;
    *p++ = ((crc)>>8)&0xffu;
    *p++ = ((crc)>>0)&0xffu;

    // IEND chunk
    *p++ = 0;       // zero payload
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 'I';     // ID
    *p++ = 'E';
    *p++ = 'N';
    *p++ = 'D';
    *p++ = 174;     // CRC
    *p++ = 66;
    *p++ = 96;
    *p++ = 130;

    *png_ptr = png;
    *p_ptr = p;
    return OK;
}



int
trell_send_png_bundle( trell_sconf_t*          sconf,
                       request_rec*            r,
                       trell_dispatch_info_t*  dispatch_info,
                       const int               width,
                       const int               height,
                       const char * const      payload )
{
    if ( dispatch_info->m_base64 == 0 ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "jny This routine is not meant for sending of non-base64-encoded image data: %s:%s:%d", r->handler, __FILE__, __LINE__ );
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    char* datestring = apr_palloc( r->pool, APR_RFC822_DATE_LEN );
    apr_rfc822_date( datestring, apr_time_now() );
    apr_table_setn( r->headers_out, "Last-Modified", datestring );
    apr_table_setn( r->headers_out, "Cache-Control", "no-cache" );

    // Encode png as base64 and send as string
    apr_table_setn( r->headers_out, "Content-Type", "text/plain" );
    ap_set_content_type( r, "text/plain" );

    struct apr_bucket_brigade* bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );

    // ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "jny Adding prefix '%s', midfix '%s' and suffix '%s'", s1, s2, s5);

    char *payload_pos = (char *)payload;

    // @@@ Why doesn't it work when we use "locally scoped" variables for strings below, when we use 'apr_bucket_transient_create'?!

    // Header in front of the rgb-image
//    {
        char *s = "{ \"rgb\": \"";
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( s, strlen(s), bb->bucket_alloc ) );
//    }

    // Base64-encoded rgb-image
//    {
        unsigned char *png, *p;
        const int tmp = trell_png_encode( sconf, r, dispatch_info, width, height, payload_pos, &png, &p );
        if (tmp!=OK)
            return tmp;
        char* base64 = apr_palloc( r->pool, apr_base64_encode_len( p-png ) );
        int base64_size = apr_base64_encode( base64, (char*)png, p-png );
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( base64, base64_size, bb->bucket_alloc ) );
//    }

    // Header in front of the depth-image (and "footer" for previous rgb-image), typically '\", depth: \"'
//    {
        char *s2 = "\", \"depth\": \"";
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( s2, strlen(s2), bb->bucket_alloc ) );
//    }

    // Base64-encoded depth buffer
//    {
//        unsigned char *png, *p;
        payload_pos += 4*((width*height*3 + 3)/4);
        const int tmp2 = trell_png_encode( sconf, r, dispatch_info, width, height, payload_pos, &png, &p );
        if (tmp2!=OK)
            return tmp2;
        char* base642 = apr_palloc( r->pool, apr_base64_encode_len( p-png ) );
        int base64_size2 = apr_base64_encode( base642, (char*)png, p-png );
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( base642, base64_size2, bb->bucket_alloc ) );
//    }

    // View matrix
//    {
        payload_pos += 4*((width*height*3 + 3)/4);
        const float * const pp = (const float * const)payload_pos;
        char matrix_string[1000];
        const int bytes_written = snprintf(matrix_string, 1000, "\", \"view\": \"%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\"",
                                           pp[0], pp[1], pp[2], pp[3], pp[4], pp[5], pp[6], pp[7], pp[8], pp[9], pp[10], pp[11], pp[12], pp[13], pp[14], pp[15]);
        assert( bytes_written < 1000 );
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( matrix_string, bytes_written, bb->bucket_alloc ) );
//    }

    // Projection matrix
//    {
        payload_pos += 4*16;
        const float * const p2 = (const float * const)payload_pos;
        char matrix_string2[1000];
        const int bytes_written2 = snprintf(matrix_string2, 1000, ", \"proj\": \"%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\" }",
                                           p2[0], p2[1], p2[2], p2[3], p2[4], p2[5], p2[6], p2[7], p2[8], p2[9], p2[10], p2[11], p2[12], p2[13], p2[14], p2[15]);
        assert( bytes_written2 < 1000 );
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( matrix_string2, bytes_written2, bb->bucket_alloc ) );
//    }

#if 0
    // To inspect the resulting package
    struct apr_bucket *b;
    for ( b = APR_BRIGADE_FIRST(bb); b != APR_BRIGADE_SENTINEL(bb); b = APR_BUCKET_NEXT(b) ) {
        const char *buf;
        size_t bytes;
        apr_bucket_read(b, &buf, &bytes, APR_BLOCK_READ);
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "jny bucket content (%lu bytes): '%s'", bytes, buf);
    }
#endif

    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );
    apr_status_t rv = ap_pass_brigade( r->output_filters, bb );
    dispatch_info->m_png_exit = apr_time_now();
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Output error" );
        return HTTP_INTERNAL_SERVER_ERROR;
    } else {
        return OK;
    }
}
