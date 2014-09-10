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
                  size_t unfiltered_offset,
                  unsigned char **dst_ptr ) // *dst_ptr will be updated
{

    trell_encode_png_state_t* encoder_state = (trell_encode_png_state_t*)data;

    int j;
    int width = encoder_state->width;
    int height = encoder_state->height;

    // apply png filter
    char* filtered   = encoder_state->filtered;
    char* unfiltered = encoder_state->buffer + unfiltered_offset;

    encoder_state->dispatch_info->m_png_filter_entry = apr_time_now();
    // We use png filter "none", and do a vertical flipping of the image
    for( j=0; j<height; j++ ) {
        filtered[ (3*width+1)*j + 0 ] = 0;
        memcpy( filtered + (3*width+1)*j + 1, unfiltered + 3*width*(height-j-1), width*3 );
    }
    encoder_state->dispatch_info->m_png_filter_exit = apr_time_now();

    uLong bound = compressBound( (3*width+1)*height );
    // unsigned char* png = apr_palloc( encoder_state->r->pool, bound + 8 + 25 + 12 + 12 + 12 );
    // unsigned char* p = png;
    unsigned char* p = *dst_ptr;

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

    *dst_ptr = p;
    return OK;
}


static
int
trell_pass_reply_png_bundle( void*          data,
                             const char    *buffer,
                             const size_t   buffer_bytes,
                             const int      part,
                             const int      more,
                             const char * const viewer_key_list );


static
int
trell_pass_reply_png_bundle_wo_depth( void*          data,
                                      const char    *buffer,
                                      const size_t   buffer_bytes,
                                      const int      part,
                                      const int      more,
                                      const char * const viewer_key_list );


int
trell_pass_reply_png( void* data,
                      const char *buffer,
                      const size_t buffer_bytes,
                      const int part,
                      const int more )
{
    trell_encode_png_state_t* encoder_state = (trell_encode_png_state_t*)data;

    if ( encoder_state->dispatch_info->m_pixel_format == TRELL_PIXEL_FORMAT_RGB_CUSTOM_DEPTH ) {
        // We escape into the new routine packaging both rgb, depth and transformation data
        return trell_pass_reply_png_bundle( data, buffer, buffer_bytes, part, more, encoder_state->dispatch_info->m_viewer_key_list );
    }

    if ( encoder_state->dispatch_info->m_pixel_format == TRELL_PIXEL_FORMAT_RGB ) {
        // We escape into the new routine packaging only rgb data, but for a list of viewers
        return trell_pass_reply_png_bundle_wo_depth( data, buffer, buffer_bytes, part, more, encoder_state->dispatch_info->m_viewer_key_list );
    }

    // ... falling back to old routine. Should probably never end up here from now on. (140908)

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




static int trell_pass_reply_png_bundle( void*          data,
                                        const char    *buffer,
                                        const size_t   buffer_bytes,
                                        const int      part,
                                        const int      more,
                                        const char * const viewer_key_list )
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

    static int num_of_keys = 0;
    size_t buffer_img_size=0, padded_img_size=0, matrix_size = sizeof(float)*16, canvas_size;

    size_t offset = 0;
    if( part == 0 ) {

        // Figuring out the number of keys, so that we can allocate appropriate buffers
        {
            num_of_keys = 1;
            const char *p = viewer_key_list;
            while ( (p-viewer_key_list<TRELL_VIEWER_KEY_LIST_MAXLENGTH) && (*p!=0) ) {
                num_of_keys += ( *p == ',' );
                p++;
            }
            ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, encoder_state->r, "XXXXXXXXXXXX %d", num_of_keys );
            if ( (*viewer_key_list==',') /* || ... other sensible tests to do? */ ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "Invalid list of viewer keys: %s", viewer_key_list );
                return -1; // error
            }
        }

        encoder_state->dispatch_info->m_png_entry = apr_time_now();
        // first invocation
        tinia_msg_image_t* msg = (tinia_msg_image_t*)buffer;
        if( msg->msg.type != TRELL_MESSAGE_IMAGE ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "got reply of type %d.", msg->msg.type );
            return -1; // error
        }
        encoder_state->width  = msg->width;
        encoder_state->height = msg->height;
        buffer_img_size       = 3 * encoder_state->width * encoder_state->height;
        padded_img_size       = 4*( (buffer_img_size+3)/4 );
        canvas_size           = 2*padded_img_size + 2*matrix_size;
        encoder_state->buffer = apr_palloc( encoder_state->r->pool, num_of_keys * canvas_size );
        const size_t filtered_img_size_bound = (3*encoder_state->width+1) * encoder_state->height; // The +1 is for the png filter flag
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
        const size_t bytes_expected = num_of_keys*canvas_size;
        if( encoder_state->bytes_read != bytes_expected ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "trell_pass_reply_png_bundle: expected %d bytes, got %ld bytes.",
                           (int)bytes_expected, encoder_state->bytes_read );
            return -1;
        }

        uLong bound = compressBound( (3*encoder_state->width+1)*encoder_state->height );
        const size_t total_bound = bound + 8 + 25 + 12 + 12 + 12 + 2*sizeof(float)*16; // misforståelse her også?
        unsigned char* png = apr_palloc( encoder_state->r->pool, total_bound );
        unsigned char* p = png;

        char* datestring = apr_palloc( encoder_state->r->pool, APR_RFC822_DATE_LEN );
        apr_rfc822_date( datestring, apr_time_now() );
        apr_table_setn( encoder_state->r->headers_out, "Last-Modified", datestring );
        apr_table_setn( encoder_state->r->headers_out, "Cache-Control", "no-cache" );

        // Encode png as base64 and send as string
        apr_table_setn( encoder_state->r->headers_out, "Content-Type", "text/plain" );
        ap_set_content_type( encoder_state->r, "text/plain" );

        struct apr_bucket_brigade* bb = apr_brigade_create( encoder_state->r->pool, encoder_state->r->connection->bucket_alloc );


        int i;
#if 0
        static int cntr2=0;
        char fname[1000];
#endif

        char vkl_copy[TRELL_VIEWER_KEY_LIST_MAXLENGTH];
        memcpy( vkl_copy, viewer_key_list, TRELL_VIEWER_KEY_LIST_MAXLENGTH );
        const char * next_key = strtok( vkl_copy, "," );

        BB_APPEND_STRING( encoder_state->r->pool, bb, "{ " );
        for (i=0; i<num_of_keys; i++) {
            BB_APPEND_STRING( encoder_state->r->pool, bb, "%s: { \"rgb\": \"", next_key );
            {
                p = png; // Reusing the old buffer, should be ok when we use the "transient" buckets that copy data.
#if 0
                // Writing out the images grabbed with readpixels() somewhere (IPCGLJobController.cpp probably)
                {
                    sprintf(fname, "/tmp/png_rgb_%05d.ppm", cntr2);
                    FILE *fp = fopen(fname, "w");
                    fprintf(fp, "P6\n%d\n%d\n255\n", encoder_state->width, encoder_state->height);
                    fwrite(encoder_state->buffer + i*canvas_size, 1, 3*encoder_state->width*encoder_state->height, fp);
                    fclose(fp);
                }
                {
                    sprintf(fname, "/tmp/png_depth_%05d.ppm", cntr2);
                    FILE *fp = fopen(fname, "w");
                    fprintf(fp, "P6\n%d\n%d\n255\n", encoder_state->width, encoder_state->height);
                    fwrite(encoder_state->buffer + i*canvas_size + padded_img_size, 1, 3*encoder_state->width*encoder_state->height, fp);
                    fclose(fp);
                }
#endif
                int rv = trell_png_encode( data, i*canvas_size, &p );
                if ( p-png > total_bound ) {
                    // @@@ This test should not be needed, the encoding routine checks this
                    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "trell_pass_reply_png_bundle: encoder has overrun the buffer!" );
                    return -1;
                }
#if 0
                {
                    // Writing out the image immediately after png-encoding
                    // This image gets corrupted occasionally! @@@
                    sprintf(fname, "/tmp/png_rgb2_%05d.png", cntr2);
                    FILE *fp = fopen(fname, "w");
                    fwrite(png, 1, p-png, fp);
                    fclose(fp);
                }
#endif
                if (rv!=OK)
                    return rv;
                char* base64 = apr_palloc( encoder_state->r->pool, apr_base64_encode_len( p-png ) );
                int base64_size = apr_base64_encode( base64, (char*)png, p-png );
                // Seems like the zero-byte is included in the string size.
                if( (base64_size > 0) && (base64[base64_size-1] == '\0') ) {
                    base64_size--;
                }
                APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( base64, base64_size, bb->bucket_alloc ) );
            }
            BB_APPEND_STRING( encoder_state->r->pool, bb, "\", \"depth\": \"" );
            {
                p = png; // Reusing the old buffer, should be ok when we use the "transient" buckets that copy data.
                int rv = trell_png_encode( data, i*canvas_size + padded_img_size , &p );
                if ( p-png > total_bound ) {
                    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "trell_pass_reply_png_bundle: encoder has overrun the buffer!" );
                    return -1;
                }
#if 0
                {
                    // This image gets corrupted occasionally! @@@
                    sprintf(fname, "/tmp/png_depth2_%05d.png", cntr2);
                    FILE *fp = fopen(fname, "w");
                    fwrite(png, 1, p-png, fp);
                    fclose(fp);
                }
#endif
                if (rv!=OK)
                    return rv;
                char* base64 = apr_palloc( encoder_state->r->pool, apr_base64_encode_len( p-png ) );
                int base64_size = apr_base64_encode( base64, (char*)png, p-png );
                // Seems like the zero-byte is included in the string size.
                if( (base64_size > 0) && (base64[base64_size-1] == '\0') ) {
                    base64_size--;
                }
                APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( base64, base64_size, bb->bucket_alloc ) );
            }
            const float * const MV = (const float * const)( encoder_state->buffer + i*canvas_size + 2*padded_img_size );
            BB_APPEND_STRING( encoder_state->r->pool, bb, "\", view: \"%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\"",
                              MV[0], MV[1], MV[2], MV[3], MV[4], MV[5], MV[6], MV[7], MV[8], MV[9], MV[10], MV[11], MV[12], MV[13], MV[14], MV[15] );
            const float * const PM = (const float * const)( encoder_state->buffer + i*canvas_size + 2*padded_img_size + sizeof(float)*16 );
            BB_APPEND_STRING( encoder_state->r->pool, bb, ", proj: \"%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\" }",
                              PM[0], PM[1], PM[2], PM[3], PM[4], PM[5], PM[6], PM[7], PM[8], PM[9], PM[10], PM[11], PM[12], PM[13], PM[14], PM[15] );
            if (i<num_of_keys-1) {
                BB_APPEND_STRING( encoder_state->r->pool, bb, ", " );
            }
#if 0
            {
                sprintf(fname, "/tmp/png_rgb2_%05d.ppm", cntr2);
                FILE *fp = fopen(fname, "w");
                fprintf(fp, "P6\n%d\n%d\n255\n", encoder_state->width, encoder_state->height);
                fwrite(encoder_state->buffer + i*canvas_size, 1, 3*encoder_state->width*encoder_state->height, fp);
                fclose(fp);
            }
            {
                sprintf(fname, "/tmp/png_depth2_%05d.ppm", cntr2);
                FILE *fp = fopen(fname, "w");
                fprintf(fp, "P6\n%d\n%d\n255\n", encoder_state->width, encoder_state->height);
                fwrite(encoder_state->buffer + i*canvas_size + padded_img_size, 1, 3*encoder_state->width*encoder_state->height, fp);
                fclose(fp);
            }
            cntr2++;
#endif
            next_key = strtok( NULL, "," );
            if (   ( (i<num_of_keys-1) && (next_key==NULL) )   ||   ( (i==num_of_keys-1) && (next_key!=NULL) )   ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "trell_pass_reply_png_bundle: strtok has not worked as expected. Problem with the viewer_key_list? (%s)", viewer_key_list );
                return -1;
            }
        }
        BB_APPEND_STRING( encoder_state->r->pool, bb, " }" );


#if 0
        // To inspect the resulting package, see the apache error log
        struct apr_bucket *b;
        for ( b = APR_BRIGADE_FIRST(bb); b != APR_BRIGADE_SENTINEL(bb); b = APR_BUCKET_NEXT(b) ) {
            const char *buf;
            size_t bytes;
            apr_bucket_read(b, &buf, &bytes, APR_BLOCK_READ);
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "bucket content (%lu bytes): '%s'", bytes, buf);
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




static int trell_pass_reply_png_bundle_wo_depth( void*          data,
                                                 const char    *buffer,
                                                 const size_t   buffer_bytes,
                                                 const int      part,
                                                 const int      more,
                                                 const char * const viewer_key_list )
{
    const int w_depth = 0;

    trell_encode_png_state_t* encoder_state = (trell_encode_png_state_t*)data;

    // ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "trell_pass_reply_png_bundle_wo_depth: key=%s", encoder_state->dispatch_info->m_key );
    // This key is not the "list of keys", so we cannot use it to detect how many images we are to encode.

    if ( encoder_state->dispatch_info->m_base64 == 0 ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r,
                       "This routine is not meant for sending of non-base64-encoded image data: %s:%s:%d",
                       encoder_state->r->handler, __FILE__, __LINE__ );
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    static int num_of_keys = 0;
    size_t buffer_img_size=0, padded_img_size=0, matrix_size = sizeof(float)*16*w_depth, canvas_size;

    size_t offset = 0;
    if( part == 0 ) {

        // Figuring out the number of keys, so that we can allocate appropriate buffers
        {
            num_of_keys = 1;
            const char *p = viewer_key_list;
            while ( (p-viewer_key_list<TRELL_VIEWER_KEY_LIST_MAXLENGTH) && (*p!=0) ) {
                num_of_keys += ( *p == ',' );
                p++;
            }
            ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, encoder_state->r, "XXXXXXXXXXXX 2 %d", num_of_keys );
            if ( (*viewer_key_list==',') /* || ... other sensible tests to do? */ ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "Invalid list of viewer keys: %s", viewer_key_list );
                return -1; // error
            }
        }

        encoder_state->dispatch_info->m_png_entry = apr_time_now();
        // first invocation
        tinia_msg_image_t* msg = (tinia_msg_image_t*)buffer;
        if( msg->msg.type != TRELL_MESSAGE_IMAGE ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "got reply of type %d.", msg->msg.type );
            return -1; // error
        }
        encoder_state->width  = msg->width;
        encoder_state->height = msg->height;
        buffer_img_size       = 3 * encoder_state->width * encoder_state->height;
        padded_img_size       = 4*( (buffer_img_size+3)/4 );
        canvas_size           = padded_img_size + w_depth*( padded_img_size + 2*matrix_size );
        encoder_state->buffer = apr_palloc( encoder_state->r->pool, num_of_keys * canvas_size );
        const size_t filtered_img_size_bound = (3*encoder_state->width+1) * encoder_state->height; // The +1 is for the png filter flag
        encoder_state->filtered = apr_palloc( encoder_state->r->pool, filtered_img_size_bound + w_depth*2*matrix_size ); // Just in case the image is smaller than 4*16 bytes!

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
        const size_t bytes_expected = num_of_keys*canvas_size;
        if( encoder_state->bytes_read != bytes_expected ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "trell_pass_reply_png_bundle_wo_depth: expected %d bytes, got %ld bytes.",
                           (int)bytes_expected, encoder_state->bytes_read );
            return -1;
        }

        uLong bound = compressBound( (3*encoder_state->width+1)*encoder_state->height );
        const size_t total_bound = bound + 8 + 25 + 12 + 12 + 12 + w_depth*2*sizeof(float)*16; // misforståelse her også?
        unsigned char* png = apr_palloc( encoder_state->r->pool, total_bound );
        unsigned char* p = png;

        char* datestring = apr_palloc( encoder_state->r->pool, APR_RFC822_DATE_LEN );
        apr_rfc822_date( datestring, apr_time_now() );
        apr_table_setn( encoder_state->r->headers_out, "Last-Modified", datestring );
        apr_table_setn( encoder_state->r->headers_out, "Cache-Control", "no-cache" );

        // Encode png as base64 and send as string
        apr_table_setn( encoder_state->r->headers_out, "Content-Type", "text/plain" );
        ap_set_content_type( encoder_state->r, "text/plain" );

        struct apr_bucket_brigade* bb = apr_brigade_create( encoder_state->r->pool, encoder_state->r->connection->bucket_alloc );


        int i;
#if 0
        static int cntr2=0;
        char fname[1000];
#endif

        char vkl_copy[TRELL_VIEWER_KEY_LIST_MAXLENGTH];
        memcpy( vkl_copy, viewer_key_list, TRELL_VIEWER_KEY_LIST_MAXLENGTH );
        const char * next_key = strtok( vkl_copy, "," );

        BB_APPEND_STRING( encoder_state->r->pool, bb, "{ " );
        for (i=0; i<num_of_keys; i++) {
            BB_APPEND_STRING( encoder_state->r->pool, bb, "%s: { \"rgb\": \"", next_key );
            {
                p = png; // Reusing the old buffer, should be ok when we use the "transient" buckets that copy data.
#if 0
                // Writing out the images grabbed with readpixels() somewhere (IPCGLJobController.cpp probably)
                {
                    sprintf(fname, "/tmp/png_rgb_%05d.ppm", cntr2);
                    FILE *fp = fopen(fname, "w");
                    fprintf(fp, "P6\n%d\n%d\n255\n", encoder_state->width, encoder_state->height);
                    fwrite(encoder_state->buffer + i*canvas_size, 1, 3*encoder_state->width*encoder_state->height, fp);
                    fclose(fp);
                }
#endif
                int rv = trell_png_encode( data, i*canvas_size, &p );
                if ( p-png > total_bound ) {
                    // @@@ This test should not be needed, the encoding routine checks this
                    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "trell_pass_reply_png_bundle_wo_depth: encoder has overrun the buffer!" );
                    return -1;
                }
#if 0
                {
                    // Writing out the image immediately after png-encoding
                    // This image gets corrupted occasionally! @@@
                    sprintf(fname, "/tmp/png_rgb2_%05d.png", cntr2);
                    FILE *fp = fopen(fname, "w");
                    fwrite(png, 1, p-png, fp);
                    fclose(fp);
                }
#endif
                if (rv!=OK)
                    return rv;
                char* base64 = apr_palloc( encoder_state->r->pool, apr_base64_encode_len( p-png ) );
                int base64_size = apr_base64_encode( base64, (char*)png, p-png );
                // Seems like the zero-byte is included in the string size.
                if( (base64_size > 0) && (base64[base64_size-1] == '\0') ) {
                    base64_size--;
                }
                APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( base64, base64_size, bb->bucket_alloc ) );
            }
            BB_APPEND_STRING( encoder_state->r->pool, bb, "\" }" );
            if (i<num_of_keys-1) {
                BB_APPEND_STRING( encoder_state->r->pool, bb, ", " );
            }
#if 0
            {
                sprintf(fname, "/tmp/png_rgb2_%05d.ppm", cntr2);
                FILE *fp = fopen(fname, "w");
                fprintf(fp, "P6\n%d\n%d\n255\n", encoder_state->width, encoder_state->height);
                fwrite(encoder_state->buffer + i*canvas_size, 1, 3*encoder_state->width*encoder_state->height, fp);
                fclose(fp);
            }
            cntr2++;
#endif
            next_key = strtok( NULL, "," );
            if (   ( (i<num_of_keys-1) && (next_key==NULL) )   ||   ( (i==num_of_keys-1) && (next_key!=NULL) )   ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "trell_pass_reply_png_bundle_wo_depth: strtok has not worked as expected. Problem with the viewer_key_list? (%s)", viewer_key_list );
                return -1;
            }
        }
        BB_APPEND_STRING( encoder_state->r->pool, bb, " }" );


#if 1
        // To inspect the resulting package, see the apache error log
        struct apr_bucket *b;
        for ( b = APR_BRIGADE_FIRST(bb); b != APR_BRIGADE_SENTINEL(bb); b = APR_BUCKET_NEXT(b) ) {
            const char *buf;
            size_t bytes;
            apr_bucket_read(b, &buf, &bytes, APR_BLOCK_READ);
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "jny trell_pass_reply_png_bundle_wo_depth bucket content (%lu bytes): '%s'", bytes, buf);
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
