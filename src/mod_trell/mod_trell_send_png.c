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

    // Apply an PNG prediction filter, and do it upside-down effectively
    // flipping the image.

    char* filtered = apr_palloc( r->pool, (3*width+1)*height );
    for( j=0; j<height; j++) {
        filtered[ (3*width+1)*j + 0 ] = 0;
        for(i=0; i<width; i++) {
            filtered[ (3*width+1)*j + 1 + 3*i + 0 ] = payload[ 3*width*(height-j-1) + 3*i + 2 ];
            filtered[ (3*width+1)*j + 1 + 3*i + 1 ] = payload[ 3*width*(height-j-1) + 3*i + 1 ];
            filtered[ (3*width+1)*j + 1 + 3*i + 2 ] = payload[ 3*width*(height-j-1) + 3*i + 0 ];
        }
    }


    uLong bound = compressBound( (3*width+1)*height );
    unsigned char* png = apr_palloc( r->pool, bound + 8 + 25 + 12 + 12 );
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

    int c = compress( (Bytef*)(p+8), &bound, (Bytef*)filtered, (3*width+1)*height );
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
    *p++ = ((crc)>>24)&0xffu;    // image width
    *p++ = ((crc)>>16)&0xffu;
    *p++ = ((crc)>>8)&0xffu;
    *p++ = ((crc)>>0)&0xffu;

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
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Output error" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    else {
        return OK;
    }
}

