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

/* -*- mode: C++; tab-width:4; c-basic-offset: 4; indent-tabs-mode:nil -*- */
/************************************************************************
 *
 *  File: DumpFrames.hpp
 *
 *  Created: 2009-08-14
 *
 *  Version:
 *
 *  Authors: Christopher Dyken <christopher.dyken@sintef.no>
 *
 *  This file is part of the siut library.
 *  Copyright (C) 2009 by SINTEF.  All rights reserved.
 *
 *  SINTEF, Pb 124 Blindern, N-0314 Oslo, Norway
 *  http://www.sintef.no
 *********************************************************************/
#pragma once

#include <string>
#include <cstring>
#include <stdlib.h>
#ifdef __unix
#include <aio.h>
#endif // of __unix


namespace siut2 {
    namespace io_utils  {

/** Asynchronously dump one frame of a series of frames as a RLE-compressed TGA file.
  *
  * This function is intended to be used to do real-time capture the contents of
  * the current OpenGL framebuffer to disc as a series of images. Dumping
  * 1280x720x24 30 times a second can be quite stressful for the IO subsystem of
  * the computer, so this functions tries to minimize the stress in the
  * following ways:
  * - The framebuffer is read back asyncronously, so the frame isn't actually
  *   stored on disc before the next time this function is invoked.
  * - The image data is run-length encoded to reduce the actual file-size. This
  *   increases the CPU load a bit, but reduces the amount of data to be stored
  *   quite drastically in most situations.
  * - (Linux only) The file is written in direct mode, trying to avoid one extra
  *   copy by the kernel. However, it seems that the payload must be a multiple
  *   of 512, so file gets padded up to the nearest multiple of 512. Neither
  *   GIMP nor mencoder seem to notice that the files might be slightly too
  *   large.
  * - (Linux only) The file is written using async disc io, so the write buffer
  *   doesn't have to be finished processing until the next frame.
  *
  * This function has to be called one time more than the number of frames that
  * should be produced. When creating movies this is usually no problem, but if
  * one wants to use this function to do screenshots, this function must be
  * called twice to make one file (one invocation to fetch the image from the
  * GPU and one invocation to store this image to disc).
  *
  * \param basename  The basename of the file, xxxxx.tga is appended to this.
  *
  */
void
dumpFrames( const std::string& basename )
{
    static bool broken = false;
    if( broken ) {
        return;
    }

    static bool first = true;
    static GLuint buffer;
    static GLsizei w = 0;
    static GLsizei h = 0;
    static GLubyte* mem = NULL;
    static int frame_no=0;

#ifdef __unix
    static int my_fd = -1;
    static aiocb my_aiocb;
#endif

    if( first ) {
        glGenBuffers( 1, &buffer );
        first = false;
    }
    // process previous frame
    else  {

        glBindBuffer( GL_PIXEL_PACK_BUFFER, buffer );
        GLubyte __attribute__((aligned(16))) *frame =
                reinterpret_cast<GLubyte*>( glMapBuffer( GL_PIXEL_PACK_BUFFER,
                                                         GL_READ_ONLY ) );
        if( (unsigned long)frame & 0xf ) {
            std::cerr
                    << "siut::io_utils::dumpFrames(): "
                    << "Pointer from glMapBuffer is not aligned. "
                    << "Subsequent calls will have no effect."
                    << std::endl;
            glUnmapBuffer( GL_PIXEL_PACK_BUFFER );
            glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
            broken = true;
            return;
        }

#ifdef __unix
        GLubyte __attribute__((aligned(16))) *p = mem;
#else
        GLubyte *p = mem;
#endif

        // TGA header
        *p++ = 0;                // id length
        *p++ = 0;                // color map type
        *p++ = 10;               // data type code
        *p++ = 0;                // color map origin LSB
        *p++ = 0;                // color map origin MSB
        *p++ = 0;                // color map length LSB
        *p++ = 0;                // color map length MSB
        *p++ = 0;                // color map depth
        *p++ = 0;                // x origin LSB
        *p++ = 0;                // x origin MSB
        *p++ = 0;                // y origin LSB
        *p++ = 0;                // y origin MSB
        *p++ = w & 0xffu;     // width LSB
        *p++ = (w>>8) & 0xffu;   // width MSB
        *p++ = h & 0xffu;        // height LSB
        *p++ = (h>>8) & 0xffu;   // height MSB
        *p++ = 24u;              // bits per pixel
        *p++ = 0;                // image descriptor

        // run-length encode image data
        for(int y=0; y<h; y++) {
            // encode one scanline
            GLubyte* l = &frame[ 3*w*(y) ];
            GLubyte* r = &frame[ 3*w*(y+1) ];
            while( l < r ) {
                // build one packet
                unsigned int prev_pix = (l[2]<<16) | (l[1]<<8) | l[0];

                *p++ = 0;
                *p++ = l[2];
                *p++ = l[1];
                *p++ = l[0];
                // First, try to build a RLE packet
                GLubyte*c;
                for( c=l+3; (c<r) && (c-l < 3*128); c+=3) {
                    unsigned int curr_pix = (c[2]<<16) | (c[1]<<8) | c[0];
                    if( prev_pix != curr_pix )
                        break;
                }
                // Something to compress, opt for RLE-packet
                if( c-l > 3 ) {
                    // store no. repetitions
                    p[ -4 ] = ((c-l)/3-1) | 128;
                    l = c;
                }
                // Nothing to compress, make non-RLE-packet
                else {
                // search until end of scanline and packet for possible RLE packet
                    for( c=l+3; (c<r) &&
                                (c-l < 3*128) &&
                                (!((c[-3] == c[0]) &&
                                   (c[-2] == c[1]) &&
                                   (c[-1] == c[2])) ); c+=3) {
                        *p++ = c[2];
                        *p++ = c[1];
                        *p++ = c[0];
                   }
                    // store non-RLE-packet size
                    p[ -(c-l) -1 ] = (c-l)/3u-1u;
                    l = c;
                }
            }
        }
        glUnmapBuffer( GL_PIXEL_PACK_BUFFER );
        glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );

        // create new filename
        char filename[512];
        snprintf( &filename[0], 512, "%s%05d.tga",
                  basename.c_str(), frame_no++ );

        // --- submit tga file image to disc
#ifdef __unix__buggy_linux
        // wait for completion of writing the previous frame and close file
        if( my_fd != -1 ) {
            bzero( reinterpret_cast<char*>( &my_aiocb ), sizeof( struct aiocb ) );
            my_aiocb.aio_fildes = my_fd;
            if( aio_fsync( O_SYNC, &my_aiocb ) == -1 ) {
                std::cerr
                        << "siut::io_utils::dumpFrames(): "
                        << "Synchronization of async file handle failed. "
                        << "Subsequent calls will have no effect."
                        << std::endl;
                broken = true;
                return;
            }
            if( close( my_fd ) == -1 ) {
                std::cerr
                        << "siut::io_utils::dumpFrames(): "
                        << "Failed to close image file. "
                        << "Subsequent calls will have no effect."
                        << std::endl;
                broken = true;
                return;
            }
        }

        // open file for currently processed frame
        my_fd = open( &filename[0],
                      O_CREAT|O_WRONLY|O_TRUNC /*| O_DIRECT*/,
                      S_IRUSR | S_IWUSR );
        if( my_fd == -1 ) {
            std::cerr
                    << "siut::io_utils::dumpFrames(): "
                    << "Failed to open " << filename << " for writing. "
                    << "Subsequent calls will have no effect."
                    << std::endl;
            broken = true;
            return;
        }

        // send write request
        bzero( reinterpret_cast<char*>( &my_aiocb ), sizeof( struct aiocb ) );
        my_aiocb.aio_fildes = my_fd;
        my_aiocb.aio_buf = reinterpret_cast<char*>( mem );

        // direct silently doesn't work unless multiple of 512...
        // we can get some trash at end of file
        // my_aiocb.aio_nbytes = p-mem;
        my_aiocb.aio_nbytes = 512*( ((p-mem)+511)/512 );
        if( aio_write( &my_aiocb ) == -1 ) {
             std::cerr
                    << "siut::io_utils::dumpFrames(): "
                    << "Failed to issue async write request. "
                    << "Subsequent calls will have no effect."
                    << std::endl;
            broken = true;
            return;
        }


#else
#warning Async file IO missing on this platform, using synchronized IO
        std::ofstream dump ( &filename[0], std::ios::out | std::ios::trunc );
        dump.write( reinterpret_cast<char*>( mem ), p-mem );
#endif
    }

    // trigger read back of current frame
    GLsizei viewport[4];
    glGetIntegerv( GL_VIEWPORT, &viewport[0] );

    glBindBuffer( GL_PIXEL_PACK_BUFFER, buffer );
    if( first || viewport[2] != w || viewport[3] != h ) {
        w = viewport[2];
        h = viewport[3];
        // resize readback buffer
        glBufferData( GL_PIXEL_PACK_BUFFER,
                      sizeof(GLubyte)*3*w*h,
                      NULL,
                      GL_STREAM_READ );
        if( mem != NULL ) {
            free( mem );
        }
        // resize buffer to hold TGA image
#ifdef __unix
        if( posix_memalign( reinterpret_cast<void**>( &mem),
                            512,
                            512*( (sizeof(GLubyte)*(18+3*w*h)+511)/512 ) ) ) {
             std::cerr
                    << "siut::io_utils::dumpFrames(): "
                    << "Failed to issue async write request. "
                    << "Subsequent calls will have no effect."
                    << std::endl;
            broken = true;
            return;
        }
#else
        mem = reinterpret_cast<GLubyte*>( malloc( (sizeof(GLubyte)*(18+3*w*h) ) );
        if( mem == NULL ) {
             std::cerr
                    << "siut::io_utils::dumpFrames(): "
                    << "Failed to allocate memory. "
                    << "Subsequent calls will have no effect."
                    << std::endl;
            broken = true;
            return;
         }
#endif
    }
    glReadPixels( viewport[0], viewport[1],
                  viewport[2], viewport[3],
                  GL_RGB, GL_UNSIGNED_BYTE,
                  NULL );
    glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
}

    } // of namespace io_utils
} // of namespace siut
#endif // of _SIUT_IO_DUMPFRAMES_HPP_
