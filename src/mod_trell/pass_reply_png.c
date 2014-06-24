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

#include <apr_base64.h>

#include "mod_trell.h"
#include "tinia/trell/trell.h"



static
unsigned int
trell_png_crc( const unsigned int* crc_table, unsigned char* p, size_t length )
{
    size_t i;
    unsigned int crc = 0xffffffffl;
    for(i=0; i<length; i++) {
        unsigned int ix = ( p[i]^crc ) & 0xff;
        crc = crc_table[ix]^((crc>>8));
    }
    return ~crc;
}


static
int
trell_png_encode( void* data,
                  size_t offset,
                  unsigned char **p_ptr ) // @@@ Used both for input and output
{

    trell_encode_png_state_t* encoder_state = (trell_encode_png_state_t*)data;

    int i,j;
    int width = encoder_state->width;
    int height = encoder_state->height;

    // apply png filter
    char* filtered = encoder_state->filtered;
    char* unfiltered = encoder_state->buffer + offset;

    encoder_state->dispatch_info->m_png_filter_entry = apr_time_now();
    for( j=0; j<height; j++ ) {
        filtered[ 3*(width+1)*j + 0 ] = 0;
        for( i=0; i<width; i++ ) {
            filtered[ (3*width+1)*j + 1 + 3*i + 0 ] = unfiltered[ 3*width*(height-j-1) + 3*i + 2 ];
            filtered[ (3*width+1)*j + 1 + 3*i + 1 ] = unfiltered[ 3*width*(height-j-1) + 3*i + 1 ];
            filtered[ (3*width+1)*j + 1 + 3*i + 2 ] = unfiltered[ 3*width*(height-j-1) + 3*i + 0 ];
        }
    }
    encoder_state->dispatch_info->m_png_filter_exit = apr_time_now();

    uLong bound = compressBound( (3*width+1)*height );
    // unsigned char* png = apr_palloc( encoder_state->r->pool, bound + 8 + 25 + 12 + 12 + 12 );
    // unsigned char* p = png;
    unsigned char* p = *p_ptr;

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
    unsigned int crc = trell_png_crc( encoder_state->sconf->m_crc_table, p-13-4, 13+4 );
    *p++ = ((crc)>>24)&0xffu;    // image width
    *p++ = ((crc)>>16)&0xffu;
    *p++ = ((crc)>>8)&0xffu;
    *p++ = ((crc)>>0)&0xffu;

    // IDAT chunk, 12 + payload bytes in total.

    encoder_state->dispatch_info->m_png_compress_entry = apr_time_now();
    int c = compress( (Bytef*)(p+8), &bound, (Bytef*)filtered, (3*width+1)*height );
    encoder_state->dispatch_info->m_png_compress_exit = apr_time_now();
    if( c == Z_MEM_ERROR ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "Z_MEM_ERROR" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    else if( c == Z_BUF_ERROR ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "Z_BUF_ERROR" );
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
    crc = trell_png_crc( encoder_state->sconf->m_crc_table, p-bound-4, bound+4 );
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

    *p_ptr = p;
    return OK;
}


// @@@
static
int
trell_pass_reply_png_bundle( void*          data,
                             const char    *buffer, // @@@ Is data copied from 'data' to 'buffer', then again to 'filtered' during encoding?
                             const size_t   buffer_bytes, // @@@ Seems to be the number of bytes in the "binary blob", either one png or the whole bundle
                             const int      part,
                             const int      more );


int
trell_pass_reply_png( void* data,
                      const char *buffer,
                      const size_t buffer_bytes,
                      const int part,
                      const int more )
{
    trell_encode_png_state_t* encoder_state = (trell_encode_png_state_t*)data;

    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "jny er her 2, format = %d", encoder_state->dispatch_info->m_pixel_format );

    // By testing on this here, we can keep as much of the controlling code ignorant of these differences as possible. // @@@
    if ( encoder_state->dispatch_info->m_pixel_format == TRELL_PIXEL_FORMAT_BGR8_CUSTOM_DEPTH ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "jny er her" );
        return trell_pass_reply_png_bundle( data, buffer, buffer_bytes, part, more );
    }

    size_t offset = 0;
    if( part == 0 ) {
        encoder_state->dispatch_info->m_png_entry = apr_time_now();
        // first invocation
        tinia_msg_image_t* msg = (tinia_msg_image_t*)buffer;
        if( msg->msg.type != TRELL_MESSAGE_IMAGE ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r,
                           "got reply of type %d.", msg->msg.type );
            return -1; // error
        }

        encoder_state->width  = msg->width;
        encoder_state->height = msg->height;
        encoder_state->buffer = apr_palloc( encoder_state->r->pool,
                                            3*encoder_state->width*encoder_state->height );
        encoder_state->filtered = apr_palloc( encoder_state->r->pool,
                                              (3*encoder_state->width+1)*encoder_state->height );
        encoder_state->bytes_read = 0;

        offset += sizeof(tinia_msg_image_t);
    }

    if( offset < buffer_bytes ) {
        // we have data to copy.
        size_t bytes = buffer_bytes - offset;
        memcpy( encoder_state->buffer + encoder_state->bytes_read,
                buffer + offset,
                bytes );
        encoder_state->bytes_read += bytes;

    }

    if( more == 0 ) {
        // last invocation
        if( encoder_state->bytes_read != 3*encoder_state->width*encoder_state->height ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r,
                           "expected %d bytes, got %ld bytes.",
                           3*encoder_state->width*encoder_state->height,
                           encoder_state->bytes_read );
            return -1;
        }

        uLong bound = compressBound( (3*encoder_state->width+1)*encoder_state->height );
        unsigned char* png = apr_palloc( encoder_state->r->pool, bound + 8 + 25 + 12 + 12 + 12 );
        unsigned char* p = png;
        trell_png_encode( data, 0, &p ); // This updates *p

        char* datestring = apr_palloc( encoder_state->r->pool, APR_RFC822_DATE_LEN );
        apr_rfc822_date( datestring, apr_time_now() );
        apr_table_setn( encoder_state->r->headers_out, "Last-Modified", datestring );
        apr_table_setn( encoder_state->r->headers_out, "Cache-Control", "no-cache" );

        struct apr_bucket_brigade* bb = apr_brigade_create( encoder_state->r->pool,
                                                            encoder_state->r->connection->bucket_alloc );
        if( encoder_state->dispatch_info->m_base64 == 0 ) {
            // Send as plain png image
            ap_set_content_type( encoder_state->r, "image/png" );
            ap_set_content_length( encoder_state->r, p-png );
            APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_immortal_create( (char*)png, p-png, bb->bucket_alloc ) );
        }
        else {
            // Encode png as base64 and send as string
            apr_table_setn( encoder_state->r->headers_out, "Content-Type", "text/plain" );
            ap_set_content_type( encoder_state->r, "text/plain" );
            char* base64 = apr_palloc( encoder_state->r->pool, apr_base64_encode_len( p-png ) );
            int base64_size = apr_base64_encode( base64, (char*)png, p-png );
            // Seems like the zero-byte is included in the string size.
            if( (base64_size > 0) && (base64[base64_size-1] == '\0') ) {
                base64_size--;
            }
            APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_immortal_create( base64, base64_size, bb->bucket_alloc ) );
        }
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );

        apr_status_t rv = ap_pass_brigade( encoder_state->r->output_filters, bb );
        encoder_state->dispatch_info->m_png_exit = apr_time_now();

        if( rv != APR_SUCCESS ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, encoder_state->r,
                           "ap_pass_brigade failed." );
            return -1; // error
        }
    }
    return 0;   // success
    /*


        request_rec* r = encoder_state->m_r;

        tinia_msg_t* msg = (tinia_msg_t*)buffer;
        if( msg->type == TRELL_MESSAGE_IMAGE ) {
            tinia_msg_image_t* m = (tinia_msg_image_t*)buffer;
            int width = m->width;
            int height = m->height;


            char* payload = (char*)buffer + sizeof(*m);



            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->m_r,
                           "%s.pass_reply_png: w=%d, h=%d, l=%d.", encoder_state->m_r->handler,
                           width, height, (int)buffer_bytes );
            return 0;

            int i, j;

            const unsigned int* crc_table = encoder_state->m_sconf->m_crc_table;




        }
        else {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "got reply of type %d.", msg->type );
            return -1; // error
        }
        */
}


// @@@
static
int
trell_pass_reply_png_bundle( void*          data,
                             const char    *buffer, // @@@ Is data copied from 'data' to 'buffer', then again to 'filtered' during encoding?
                             const size_t   buffer_bytes, // @@@ Seems to be the number of bytes in the "binary blob", either one png or the whole bundle
                             const int      part,
                             const int      more )
{
    trell_encode_png_state_t* encoder_state = (trell_encode_png_state_t*)data;

    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "jny Entering trell_pass_reply_png_bundle...");

    if ( encoder_state->dispatch_info->m_base64 == 0 ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r,
                       "jny This routine is not meant for sending of non-base64-encoded image data: %s:%s:%d",
                       encoder_state->r->handler, __FILE__, __LINE__ );
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    size_t offset = 0;
    if( part == 0 ) {
        encoder_state->dispatch_info->m_png_entry = apr_time_now();
        // first invocation
        tinia_msg_image_t* msg = (tinia_msg_image_t*)buffer;
        if( msg->msg.type != TRELL_MESSAGE_IMAGE ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r,
                           "got reply of type %d.", msg->msg.type );
            return -1; // error
        }

        encoder_state->width  = msg->width;
        encoder_state->height = msg->height;
        const size_t buffer_img_size = 3 * encoder_state->width * encoder_state->height;
        // encoder_state->buffer = apr_palloc( encoder_state->r->pool, buffer_img_size ); // @@@ This is used for "single png"

        const size_t buffer_img_padding = 4*( (buffer_img_size+3)/4 ) - buffer_img_size;
        const size_t matrix_size = sizeof(float) * 16;
        // @@@ But now we have png + padding + png + padding + matrix + matrix
        encoder_state->buffer = apr_palloc( encoder_state->r->pool, 2*(buffer_img_size + buffer_img_padding) + 2*matrix_size );

        const size_t filtered_img_size_bound = (3*encoder_state->width+1) * encoder_state->height;
        // encoder_state->filtered = apr_palloc( encoder_state->r->pool, filtered_img_size_bound ); // @@@ This is used for "single png"
        encoder_state->filtered = apr_palloc( encoder_state->r->pool, filtered_img_size_bound + matrix_size ); // @@@ Just in case the image is smaller than 4*16 bytes!

        encoder_state->bytes_read = 0;
        offset += sizeof(tinia_msg_image_t);
    }

    if( offset < buffer_bytes ) {
        // we have data to copy.
        size_t bytes = buffer_bytes - offset;
        memcpy( encoder_state->buffer + encoder_state->bytes_read,
                buffer + offset,
                bytes );
        encoder_state->bytes_read += bytes;

    }

    if( more == 0 ) {
        // last invocation
        // const size_t bytes_expected = 3*encoder_state->width*encoder_state->height; // @@@ For single png
        const size_t buffer_img_size = 3 * encoder_state->width * encoder_state->height;
        const size_t buffer_img_padding = 4*( (buffer_img_size+3)/4 ) - buffer_img_size;
        const size_t bytes_expected = 2*(buffer_img_size + buffer_img_padding) + 2*sizeof(float)*16; // @@@ For bundle
        if( encoder_state->bytes_read != bytes_expected ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r,
                           "expected %d bytes, got %ld bytes.",
                           (int)bytes_expected,
                           encoder_state->bytes_read );
            return -1;
        }

        uLong bound = compressBound( (3*encoder_state->width+1)*encoder_state->height );
        unsigned char* png = apr_palloc( encoder_state->r->pool, bound + 8 + 25 + 12 + 12 + 12 );
        unsigned char* p = png;
//        trell_png_encode( data, &p ); // This updates *p

        char* datestring = apr_palloc( encoder_state->r->pool, APR_RFC822_DATE_LEN );
        apr_rfc822_date( datestring, apr_time_now() );
        apr_table_setn( encoder_state->r->headers_out, "Last-Modified", datestring );
        apr_table_setn( encoder_state->r->headers_out, "Cache-Control", "no-cache" );



        // Encode png as base64 and send as string
        apr_table_setn( encoder_state->r->headers_out, "Content-Type", "text/plain" );
        ap_set_content_type( encoder_state->r, "text/plain" );

        struct apr_bucket_brigade* bb = apr_brigade_create( encoder_state->r->pool, encoder_state->r->connection->bucket_alloc );

        // ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "jny Adding prefix '%s', midfix '%s' and suffix '%s'", s1, s2, s5);

        // Header in front of the rgb-image
//    {
        char *s = "{ \"rgb\": \"";
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( s, strlen(s), bb->bucket_alloc ) );
//    }

        // Base64-encoded rgb-image
//    {
        const int tmp = trell_png_encode( data, 0, &p );
        if (tmp!=OK)
            return tmp;
        char* base64 = apr_palloc( encoder_state->r->pool, apr_base64_encode_len( p-png ) );
        int base64_size = apr_base64_encode( base64, (char*)png, p-png );
        // Seems like the zero-byte is included in the string size.
        if( (base64_size > 0) && (base64[base64_size-1] == '\0') ) {
            base64_size--;
        }
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
            p = png; // @@@ Reusing the old buffer, should be ok with "transient" buckets?!
            const size_t depth_offset = 4*( (buffer_img_size+3)/4 );
            const int tmp2 = trell_png_encode( data, depth_offset, &p );
            if (tmp2!=OK)
                return tmp2;
            char* base642 = apr_palloc( encoder_state->r->pool, apr_base64_encode_len( p-png ) );
            int base64_size2 = apr_base64_encode( base642, (char*)png, p-png );
            // Seems like the zero-byte is included in the string size.
            if( (base64_size2 > 0) && (base642[base64_size2-1] == '\0') ) {
                base64_size2--;
            }
            APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( base642, base64_size2, bb->bucket_alloc ) );
//    }

        // View matrix
//    {
            const float * const pp = (const float * const)( encoder_state->buffer + 2*depth_offset );
            char matrix_string[1000];
            const int bytes_written = snprintf(matrix_string, 1000, "\", \"view\": \"%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\"",
                                               pp[0], pp[1], pp[2], pp[3], pp[4], pp[5], pp[6], pp[7], pp[8], pp[9], pp[10], pp[11], pp[12], pp[13], pp[14], pp[15]);
            assert( bytes_written < 1000 );
            APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( matrix_string, bytes_written, bb->bucket_alloc ) );
//    }

        // Projection matrix
//    {
            const float * const p2 = (const float * const)( encoder_state->buffer + 2*depth_offset + sizeof(float)*16 );
            char matrix_string2[1000];
            const int bytes_written2 = snprintf(matrix_string2, 1000, ", \"proj\": \"%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\" }",
                                               p2[0], p2[1], p2[2], p2[3], p2[4], p2[5], p2[6], p2[7], p2[8], p2[9], p2[10], p2[11], p2[12], p2[13], p2[14], p2[15]);
            assert( bytes_written2 < 1000 );
            APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( matrix_string2, bytes_written2, bb->bucket_alloc ) );
//    }

    #if 1
        // To inspect the resulting package
        struct apr_bucket *b;
        for ( b = APR_BRIGADE_FIRST(bb); b != APR_BRIGADE_SENTINEL(bb); b = APR_BUCKET_NEXT(b) ) {
            const char *buf;
            size_t bytes;
            apr_bucket_read(b, &buf, &bytes, APR_BLOCK_READ);
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "jny bucket content (%lu bytes): '%s'", bytes, buf);
        }
    #endif

        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );
        apr_status_t rv = ap_pass_brigade( encoder_state->r->output_filters, bb );


        if( rv != APR_SUCCESS ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, encoder_state->r, "ap_pass_brigade failed." );
            return -1;
        }


    }
    return 0;   // success

}

