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

#include <zlib.h>

#include <httpd.h>
#include <http_log.h>
#include <http_protocol.h>

#include <apr_base64.h>
#include <apr_strings.h>

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
                  unsigned char **p_ptr ) // Used both for input and output
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


static
int
trell_pass_reply_png_bundle( void*          data,
                             const char    *buffer,
                             const size_t   buffer_bytes,
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

    if ( encoder_state->dispatch_info->m_pixel_format == TRELL_PIXEL_FORMAT_BGR8_CUSTOM_DEPTH ) {
        // We escape into the new routine packaging both rgb, depth and transformation data
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
            char *prefix = "{ \"rgb\": \"";
            APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( prefix, strlen(prefix), bb->bucket_alloc ) );
            APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_immortal_create( base64, base64_size, bb->bucket_alloc ) );
            char *suffix = "\"}";
            APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( suffix, strlen(suffix), bb->bucket_alloc ) );
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




#define BB_APPEND_STRING( pool, bb, ...) \
{ \
    char *tmp = apr_psprintf( pool, __VA_ARGS__ ); \
    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( tmp, strlen(tmp), bb->bucket_alloc ) ); \
}




static
int
trell_pass_reply_png_bundle( void*          data,
                             const char    *buffer,
                             const size_t   buffer_bytes,
                             const int      part,
                             const int      more )
{
    trell_encode_png_state_t* encoder_state = (trell_encode_png_state_t*)data;

    // ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "trell_pass_reply_png_bundle: key=%s", encoder_state->dispatch_info->m_key );
    // This key is not the "list of keys", so we cannot use it to detect how many images we are to encode.

    if ( encoder_state->dispatch_info->m_base64 == 0 ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r,
                       "This routine is not meant for sending of non-base64-encoded image data: %s:%s:%d",
                       encoder_state->r->handler, __FILE__, __LINE__ );
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    size_t offset = 0;
    if( part == 0 ) {
        encoder_state->dispatch_info->m_png_entry = apr_time_now();
        // first invocation
        tinia_msg_image_t* msg = (tinia_msg_image_t*)buffer;
        if( msg->msg.type != TRELL_MESSAGE_IMAGE ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "got reply of type %d.", msg->msg.type );
            return -1; // error
        }
        encoder_state->width  = msg->width;
        encoder_state->height = msg->height;
        const size_t buffer_img_size = 3 * encoder_state->width * encoder_state->height;
        const size_t buffer_img_padding = 4*( (buffer_img_size+3)/4 ) - buffer_img_size;
        const size_t matrix_size = sizeof(float) * 16;
        encoder_state->buffer = apr_palloc( encoder_state->r->pool, 2 * ( 2*(buffer_img_size + buffer_img_padding) + 2*matrix_size ) );
        const size_t filtered_img_size_bound = (3*encoder_state->width+1) * encoder_state->height;
        encoder_state->filtered = apr_palloc( encoder_state->r->pool, filtered_img_size_bound + 2*matrix_size ); // Just in case the image is smaller than 4*16 bytes!

        // hmm... hvorfor var det ikke satt av plass til to filtrerte bilder over? Hvis et er nok, hvorfor var det da satt av plass til to matriser?
        // Mistenker at det er en misforståelse å ta med de to matrisene

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
        const size_t buffer_img_size = 3 * encoder_state->width * encoder_state->height; // Space for readPixel-produced packed data
        const size_t buffer_img_padding = 4*( (buffer_img_size+3)/4 ) - buffer_img_size;
        size_t bytes_expected = 2*(buffer_img_size + buffer_img_padding) + 2*sizeof(float)*16;
        bytes_expected *= 2; // We don't yet have access to the whole list of keys, and therefore not the buffer size to expect. Hardcoding 2 buffers for testing

        if( encoder_state->bytes_read != bytes_expected ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "trell_pass_reply_png_bundle: expected %d bytes, got %ld bytes.",
                           (int)bytes_expected, encoder_state->bytes_read );
            return -1;
        }

        uLong bound = compressBound( (3*encoder_state->width+1)*encoder_state->height );
        unsigned char* png = apr_palloc( encoder_state->r->pool, bound + 8 + 25 + 12 + 12 + 12 + 2*sizeof(float)*16); // misforståelse her også?
        unsigned char* p = png;

        char* datestring = apr_palloc( encoder_state->r->pool, APR_RFC822_DATE_LEN );
        apr_rfc822_date( datestring, apr_time_now() );
        apr_table_setn( encoder_state->r->headers_out, "Last-Modified", datestring );
        apr_table_setn( encoder_state->r->headers_out, "Cache-Control", "no-cache" );

        // Encode png as base64 and send as string
        apr_table_setn( encoder_state->r->headers_out, "Content-Type", "text/plain" );
        ap_set_content_type( encoder_state->r, "text/plain" );

        struct apr_bucket_brigade* bb = apr_brigade_create( encoder_state->r->pool, encoder_state->r->connection->bucket_alloc );


  //      static const char * const viewer_key_list[2] = { "viewer", "viewer2" };


        BB_APPEND_STRING( encoder_state->r->pool, bb, "{ " );


//        int i=0;
//        BB_APPEND_STRING( encoder_state->r->pool, bb, "%s: ", viewer_key_list[i] );
        BB_APPEND_STRING( encoder_state->r->pool, bb, "viewer: " );



        BB_APPEND_STRING( encoder_state->r->pool, bb, "{ \"rgb\": \"" );

        // Base64-encoded rgb-image
        int rv = trell_png_encode( data, 0, &p );
        if (rv!=OK)
            return rv;
        char* base64 = apr_palloc( encoder_state->r->pool, apr_base64_encode_len( p-png ) );
        int base64_size = apr_base64_encode( base64, (char*)png, p-png );
        // Seems like the zero-byte is included in the string size.
        if( (base64_size > 0) && (base64[base64_size-1] == '\0') ) {
            base64_size--;
        }
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( base64, base64_size, bb->bucket_alloc ) );

        BB_APPEND_STRING( encoder_state->r->pool, bb, "\", \"depth\": \"" );

        // Base64-encoded depth buffer
        p = png; // Reusing the old buffer, should be ok when we use the "transient" buckets that copy data.
        const size_t padded_img_size = 4*( (buffer_img_size+3)/4 );
        rv = trell_png_encode( data, padded_img_size, &p );
        if (rv!=OK)
            return rv;
        char* base642 = apr_palloc( encoder_state->r->pool, apr_base64_encode_len( p-png ) );
        int base64_size2 = apr_base64_encode( base642, (char*)png, p-png );
        // Seems like the zero-byte is included in the string size.
        if( (base64_size2 > 0) && (base642[base64_size2-1] == '\0') ) {
            base64_size2--;
        }
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( base642, base64_size2, bb->bucket_alloc ) );

        const float * const MV = (const float * const)( encoder_state->buffer + 2*padded_img_size );
        BB_APPEND_STRING( encoder_state->r->pool, bb, "\", view: \"%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\"",
                          MV[0], MV[1], MV[2], MV[3], MV[4], MV[5], MV[6], MV[7], MV[8], MV[9], MV[10], MV[11], MV[12], MV[13], MV[14], MV[15] );

        const float * const PM = (const float * const)( encoder_state->buffer + 2*padded_img_size + sizeof(float)*16 );
        BB_APPEND_STRING( encoder_state->r->pool, bb, ", proj: \"%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\" }",
                          PM[0], PM[1], PM[2], PM[3], PM[4], PM[5], PM[6], PM[7], PM[8], PM[9], PM[10], PM[11], PM[12], PM[13], PM[14], PM[15] );





        BB_APPEND_STRING( encoder_state->r->pool, bb, ", " );

//        BB_APPEND_STRING( encoder_state->r->pool, bb, "%s: ", viewer_key_list[i] );
        BB_APPEND_STRING( encoder_state->r->pool, bb, "viewer2: " );



        BB_APPEND_STRING( encoder_state->r->pool, bb, "{ \"rgb\": \"" );

        const size_t canvas_size = 2*padded_img_size + 2*16*sizeof(float);
        // Base64-encoded rgb-image
        p = png; // Reusing the old buffer, should be ok when we use the "transient" buckets that copy data.
        rv = trell_png_encode( data, canvas_size, &p );
        if (rv!=OK)
            return rv;
        char* base64_3 = apr_palloc( encoder_state->r->pool, apr_base64_encode_len( p-png ) );
        int base64_size_3 = apr_base64_encode( base64_3, (char*)png, p-png );
        // Seems like the zero-byte is included in the string size.
        if( (base64_size_3 > 0) && (base64_3[base64_size_3-1] == '\0') ) {
            base64_size_3--;
        }
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( base64_3, base64_size_3, bb->bucket_alloc ) );

        BB_APPEND_STRING( encoder_state->r->pool, bb, "\", \"depth\": \"" );

        // Base64-encoded depth buffer
        p = png; // Reusing the old buffer, should be ok when we use the "transient" buckets that copy data.
        // But is there a problem with the copying not being done synchronously here? I.e., there is a possibility of the next user of the buffer overwriting the content
        // before apr has done it's job?
        rv = trell_png_encode( data, canvas_size + padded_img_size, &p );
        if (rv!=OK)
            return rv;
        char* base64_4 = apr_palloc( encoder_state->r->pool, apr_base64_encode_len( p-png ) );
        int base64_size_4 = apr_base64_encode( base64_4, (char*)png, p-png );
        // Seems like the zero-byte is included in the string size.
        if( (base64_size_4 > 0) && (base64_4[base64_size_4-1] == '\0') ) {
            base64_size_4--;
        }
        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( base64_4, base64_size_4, bb->bucket_alloc ) );

        const float * const MV_2 = (const float * const)( encoder_state->buffer + canvas_size + 2*padded_img_size );
        BB_APPEND_STRING( encoder_state->r->pool, bb, "\", view: \"%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\"",
                          MV_2[0], MV_2[1], MV_2[2], MV_2[3], MV_2[4], MV_2[5], MV_2[6], MV_2[7], MV_2[8], MV_2[9], MV_2[10], MV_2[11], MV_2[12], MV_2[13], MV_2[14], MV_2[15] );

        const float * const PM_2 = (const float * const)( encoder_state->buffer + canvas_size + 2*padded_img_size + sizeof(float)*16 );
        BB_APPEND_STRING( encoder_state->r->pool, bb, ", proj: \"%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\" }",
                          PM_2[0], PM_2[1], PM_2[2], PM_2[3], PM_2[4], PM_2[5], PM_2[6], PM_2[7], PM_2[8], PM_2[9], PM_2[10], PM_2[11], PM_2[12], PM_2[13], PM_2[14], PM_2[15] );







        BB_APPEND_STRING( encoder_state->r->pool, bb, " }" );


#if 1
        // To inspect the resulting package, see the apache error log
        struct apr_bucket *b;
        for ( b = APR_BRIGADE_FIRST(bb); b != APR_BRIGADE_SENTINEL(bb); b = APR_BUCKET_NEXT(b) ) {
            const char *buf;
            size_t bytes;
            apr_bucket_read(b, &buf, &bytes, APR_BLOCK_READ);
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "jny bucket content (%lu bytes): '%s'", bytes, buf);
        }
#endif

        APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );
        apr_status_t arv = ap_pass_brigade( encoder_state->r->output_filters, bb );

        if( arv != APR_SUCCESS ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, arv, encoder_state->r, "ap_pass_brigade failed." );
            return -1;
        }

    }
    return 0;   // success

}
