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

#include <turbojpeg.h>

#include <httpd.h>
#include <http_log.h>
#include <http_protocol.h>

#include <apr_base64.h>
#include <apr_strings.h>

#include "mod_trell.h"
#include "tinia/trell/trell.h"




static int trell_jpg_encode( void *data,
                             size_t unfiltered_offset,
                             unsigned char **dst_ptr, // *dst_ptr will be updated
                             const unsigned long bound,
                             const int jpeg_quality )
{
    trell_encode_png_state_t* encoder_state = (trell_encode_png_state_t*)data;

    long unsigned jpeg_size = 0;
    unsigned char* compressed_image = NULL; //!< Memory is allocated by tjCompress2 if _jpegSize == 0
    char *buffer = encoder_state->buffer + unfiltered_offset;

    tjhandle jpeg_compressor = tjInitCompress();

    tjCompress2( jpeg_compressor,
                 (unsigned char *)buffer,
                 encoder_state->width,
                 0,
                 encoder_state->height,
                 TJPF_RGB,
                 &compressed_image,
                 &jpeg_size,                        // Initialized to 0, so that tjCompress2 will allocate memory. Resulting size returned.
                 TJSAMP_444,
                 jpeg_quality,
                 TJFLAG_FASTDCT | TJXOP_VFLIP );

    if ( jpeg_size > bound ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "trell_jpg_encode: Not enough memory reserved for compressed jpeg!" );
        return -1;
    }
    memcpy(*dst_ptr, compressed_image, jpeg_size);

    tjDestroy(jpeg_compressor);
    tjFree(compressed_image);

    *dst_ptr += jpeg_size;

    return OK;
}




#define BB_APPEND_STRING( pool, bb, ...) \
{ \
    char *tmp = apr_psprintf( pool, __VA_ARGS__ ); \
    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_transient_create( tmp, strlen(tmp), bb->bucket_alloc ) ); \
}




int
trell_pass_reply_jpg_main( void* data,
                           const char *buffer,
                           const size_t buffer_bytes,
                           const int part,
                           const int more,
                           const char * const viewer_key_list,
                           const int jpeg_quality )
{
    trell_encode_png_state_t* encoder_state = (trell_encode_png_state_t*)data;

    // ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "trell_pass_reply_jpg_main: pixel_format = %d.", encoder_state->dispatch_info->m_pixel_format );

    if ( encoder_state->dispatch_info->m_pixel_format != TRELL_PIXEL_FORMAT_RGB_JPG_VERSION ) { // @@@
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "Unknown trell pixel format: %d.", encoder_state->dispatch_info->m_pixel_format );
        return -1;
    }

    if ( encoder_state->dispatch_info->m_base64 == 0 ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r,
                       "This routine is not meant for sending of non-base64-encoded image data: %s:%s:%d",
                       encoder_state->r->handler, __FILE__, __LINE__ );
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    static int num_of_keys = 0;
    size_t buffer_img_size=0, padded_img_size=0, canvas_size=0;

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
        canvas_size           = padded_img_size;
        encoder_state->buffer = apr_palloc( encoder_state->r->pool, num_of_keys * canvas_size );
        const size_t filtered_img_size_bound = (3*encoder_state->width+1) * encoder_state->height; // The +1 is for the png filter flag
        encoder_state->filtered = apr_palloc( encoder_state->r->pool, filtered_img_size_bound ); // Just in case the image is smaller than 4*16 bytes!
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

        const unsigned long bound = (3*encoder_state->width+1)*encoder_state->height + 1000; // @@@ Just adding some, in case JPG needs it for a header or something
        const size_t total_bound = bound + 8 + 25 + 12 + 12 + 12; // @@@ Don't know where these amounts come from. encoder_state-header? png-specifics? ??
        unsigned char* png = apr_palloc( encoder_state->r->pool, total_bound );
        unsigned char* p = png;

        char* datestring = apr_palloc( encoder_state->r->pool, APR_RFC822_DATE_LEN );
        apr_rfc822_date( datestring, apr_time_now() );
        apr_table_setn( encoder_state->r->headers_out, "Last-Modified", datestring );
        apr_table_setn( encoder_state->r->headers_out, "Cache-Control", "no-cache" );

        // Encode as base64 and send as string
        apr_table_setn( encoder_state->r->headers_out, "Content-Type", "text/plain" );
        ap_set_content_type( encoder_state->r, "text/plain" );

        struct apr_bucket_brigade* bb = apr_brigade_create( encoder_state->r->pool, encoder_state->r->connection->bucket_alloc );

        int i;
        char vkl_copy[TRELL_VIEWER_KEY_LIST_MAXLENGTH];
        memcpy( vkl_copy, viewer_key_list, TRELL_VIEWER_KEY_LIST_MAXLENGTH );
        const char * next_key = strtok( vkl_copy, "," );

        BB_APPEND_STRING( encoder_state->r->pool, bb, "{ " );
        for (i=0; i<num_of_keys; i++) {
            BB_APPEND_STRING( encoder_state->r->pool, bb, "%s: { \"rgb\": \"", next_key );
            {
                p = png; // Reusing the old buffer, should be ok when we use the "transient" buckets that copy data.
                int rv = trell_jpg_encode( data, i*canvas_size, &p, total_bound, jpeg_quality );
                if ( p-png > total_bound ) {
                    // @@@ This test should not be needed, the encoding routine checks this
                    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "trell_pass_reply_png_bundle: encoder has overrun the buffer!" );
                    return -1;
                }
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
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, encoder_state->r, "bucket content (jpg version) (%lu bytes): '%s'", bytes, buf);
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




#undef BB_APPEND_STRING




int
trell_pass_reply_jpg( void* data,
                      const char *buffer,
                      const size_t buffer_bytes,
                      const int part,
                      const int more )
{
    trell_encode_png_state_t* encoder_state = (trell_encode_png_state_t*)data;
    return trell_pass_reply_jpg_main( data, buffer, buffer_bytes, part, more,
                                      encoder_state->dispatch_info->m_viewer_key_list,
                                      encoder_state->dispatch_info->m_jpeg_quality );
}
