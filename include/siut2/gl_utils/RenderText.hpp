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
 *  File: RenderText.hpp
 *
 *  Created: 2009-08-13
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

#include <GL/glew.h>

#include <string>
#include <vector>

#include "siut2/gl_utils/ContextResourceCache.hpp"

namespace siut2 {
  namespace gl_utils {


      static std::string font_8x8_desc = "0123456789.,ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz ";

      static unsigned char font_8x8_data[] = {
          ".3333..."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          ".3333..."
          "........"
          "........"
          "........"

          ".333...."
          "..33...."
          "..33...."
          "..33...."
          "..33...."
          "..33...."
          "333333.."
          "........"
          "........"
          "........"

          ".3333..."
          "....33.."
          ".3333..."
          "33......"
          "33......"
          "33......"
          "333333.."
          "........"
          "........"
          "........"

          ".3333..."
          "....33.."
          ".3333..."
          "....33.."
          "....33.."
          "....33.."
          "33333..."
          "........"
          "........"
          "........"

          "33..33.."
          "33..33.."
          "333333.."
          "....33.."
          "....33.."
          "....33.."
          "....33.."
          "........"
          "........"
          "........"

          "33333..."
          "33......"
          "33333..."
          "....33.."
          "....33.."
          "....33.."
          "33333..."
          "........"
          "........"
          "........"

          ".3333..."
          "33......"
          "33333..."
          "33..33.."
          "33..33.."
          "33..33.."
          ".3333..."
          "........"
          "........"
          "........"

          "333333.."
          "...33..."
          "..33...."
          ".33....."
          ".33....."
          ".33....."
          ".33....."
          "........"
          "........"
          "........"

          ".3333..."
          "33..33.."
          ".3333..."
          "33..33.."
          "33..33.."
          "33..33.."
          ".3333..."
          "........"
          "........"
          "........"

          ".3333..."
          "33..33.."
          ".33333.."
          "....33.."
          "....33.."
          "....33.."
          ".3333..."
          "........"
          "........"
          "........"

          "........"
          "........"
          "........"
          "........"
          "........"
          ".33....."
          ".33....."
          "........"
          "........"
          "........"

          "........"
          "........"
          "........"
          "........"
          "........"
          ".33....."
          ".33....."
          "33......"
          "........"
          "........"


          ".3333..."
          "33..33.."
          "333333.."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          "........"
          "........"
          "........"

          "33333..."
          "33..33.."
          "33333..."
          "33..33.."
          "33..33.."
          "33..33.."
          "33333..."
          "........"
          "........"
          "........"

          ".3333..."
          "33......"
          "33......"
          "33......"
          "33......"
          "33......"
          ".33333.."
          "........"
          "........"
          "........"

          "33333..."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          "33333..."
          "........"
          "........"
          "........"

          "333333.."
          "33......"
          "3333...."
          "33......"
          "33......"
          "33......"
          "333333.."
          "........"
          "........"
          "........"

          "333333.."
          "33......"
          "3333...."
          "33......"
          "33......"
          "33......"
          "33......"
          "........"
          "........"
          "........"

          ".33333.."
          "33......"
          "33.333.."
          "33..33.."
          "33..33.."
          "33..33.."
          ".33333.."
          "........"
          "........"
          "........"

          "33..33.."
          "33..33.."
          "333333.."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          "........"
          "........"
          "........"

          "333333.."
          "..33...."
          "..33...."
          "..33...."
          "..33...."
          "..33...."
          "333333.."
          "........"
          "........"
          "........"

          "..3333.."
          "....33.."
          "....33.."
          "....33.."
          "....33.."
          "....33.."
          "33333..."
          "........"
          "........"
          "........"

          "33..33.."
          "33.33..."
          "3333...."
          "33.33..."
          "33..33.."
          "33..33.."
          "33..33.."
          "........"
          "........"
          "........"

          "33......"
          "33......"
          "33......"
          "33......"
          "33......"
          "33......"
          "333333.."
          "........"
          "........"
          "........"

          "33...33."
          "333.333."
          "3333333."
          "33.3.33."
          "33.3.33."
          "33.3.33."
          "33.3.33."
          "........"
          "........"
          "........"

          "33..33.."
          "333.33.."
          "333333.."
          "33.333.."
          "33..33.."
          "33..33.."
          "33..33.."
          "........"
          "........"
          "........"

          ".3333..."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          ".3333..."
          "........"
          "........"
          "........"

          "33333..."
          "33..33.."
          "33333..."
          "33......"
          "33......"
          "33......"
          "33......"
          "........"
          "........"
          "........"

          ".3333..."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          "33.333.."
          ".33333.."
          "........"
          "........"
          "........"

          "33333..."
          "33..33.."
          "33333..."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          "........"
          "........"
          "........"

          ".3333..."
          "33......"
          ".3333..."
          "....33.."
          "....33.."
          "....33.."
          "33333..."
          "........"
          "........"
          "........"

          "333333.."
          "..33...."
          "..33...."
          "..33...."
          "..33...."
          "..33...."
          "..33...."
          "........"
          "........"
          "........"

          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          ".3333..."
          "........"
          "........"
          "........"

          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          ".3333..."
          "..33...."
          "........"
          "........"
          "........"

          "33.3.33."
          "33.3.33."
          "33.3.33."
          "33.3.33."
          "33.3.33."
          "333.333."
          "33...33."
          "........"
          "........"
          "........"

          "33..33.."
          "33..33.."
          ".3333..."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          "........"
          "........"
          "........"

          "33..33.."
          "33..33.."
          ".3333..."
          "..33...."
          "..33...."
          "..33...."
          "..33...."
          "........"
          "........"
          "........"

          ".33333.."
          "....33.."
          ".3333..."
          "33..,,.."
          "33..,,.."
          "33......"
          "333333.."
          "........"
          "........"
          "........"

          "........"
          "........"
          ".3333..."
          "....33.."
          ".33333.."
          "33..33.."
          ".33333.."
          "........"
          "........"
          "........"

          "33......"
          "33......"
          "33333..."
          "33..33.."
          "33..33.."
          "33..33.."
          "33333..."
          "........"
          "........"
          "........"

          "........"
          "........"
          ".33333.."
          "33......"
          "33......"
          "33......"
          ".33333.."
          "........"
          "........"
          "........"

          "....33.."
          "....33.."
          ".33333.."
          "33..33.."
          "33..33.."
          "33..33.."
          ".33333.."
          "........"
          "........"
          "........"

          "........"
          "........"
          ".3333..."
          "33...3.."
          "333333.."
          "33......"
          ".33333.."
          "........"
          "........"
          "........"

          "...333.."
          "..33...."
          "333333.."
          "..33...."
          "..33...."
          "..33...."
          "..33...."
          "........"
          "........"
          "........"


          "........"
          "........"
          ".33333.."
          "33..33.."
          "33..33.."
          "33..33.."
          ".33333.."
          "....33.."
          ".3333..."
          "........"

          "33......"
          "33......"
          "333331.."
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          "........"
          "........"
          "........"

          "..33...."
          "........"
          ".333...."
          "..33...."
          "..33...."
          "..33...."
          ".33333.."
          "........"
          "........"
          "........"

          "....33.."
          "........"
          "...333.."
          "....33.."
          "....33.."
          "....33.."
          "....33.."
          "....33.."
          ".3333..."
          "........"

          "33......"
          "33......"
          "33..33.."
          "33.33..."
          "3333...."
          "33.33..."
          "33..33.."
          "........"
          "........"
          "........"

          ".3333..."
          "...33..."
          "...33..."
          "...33..."
          "...33..."
          "...33..."
          "...133.."
          "........"
          "........"
          "........"

          "........"
          "........"
          "333333.."
          "3313123."
          "3313123."
          "3313123."
          "3313123."
          "........"
          "........"
          "........"

          "........"
          "........"
          "333333.."
          "33...33."
          "33...33."
          "33...33."
          "33...33."
          "........"
          "........"
          "........"

          "........"
          "........"
          ".3333..."
          "33..33.."
          "33..33.."
          "33..33.."
          ".3333..."
          "........"
          "........"
          "........"

          "........"
          "........"
          "33333..."
          "33..33.."
          "33..33.."
          "33..33.."
          "33333..."
          "33......"
          "33......"
          "........"

          "........"
          "........"
          ".33333.."
          "33..33.."
          "33..33.."
          "33..33.."
          ".33333.."
          "....33.."
          "....33.."
          "........"

          "........"
          "........"
          "33.33..."
          "333.33.."
          "33......"
          "33......"
          "33......"
          "........"
          "........"
          "........"

          "........"
          "........"
          ".33333.."
          "33......"
          ".3333..."
          "....33.."
          "33333..."
          "........"
          "........"
          "........"

         "...33...."
          "..33...."
          "333333.."
          "..33...."
          "..33...."
          "..33...."
          "...333.."
          "........"
          "........"
          "........"

          "........"
          "........"
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          ".33333.."
          "........"
          "........"
          "........"

          "........"
          "........"
          "33..33.."
          "33..33.."
          "33..33.."
          ".3333..."
          "..33...."
          "........"
          "........"
          "........"

          "........"
          "........"
          "33.3.33."
          "33.3.33."
          "33.3.33."
          "333.333."
          "33...33."
          "........"
          "........"
          "........"

          "........"
          "........"
          "33..33.."
          "33..33.."
          ".3333..."
          "33..33.."
          "33..33.."
          "........"
          "........"
          "........"

          "........"
          "........"
          "33..33.."
          "33..33.."
          "33..33.."
          "33..33.."
          ".33333.."
          "....33.."
          "33333..."
          "........"

          "........"
          "........"
          "333333.."
          "....33.."
          ".3333..."
          "33......"
          "333333.."
          "........"
          "........"
          "........"


          "........"
          "........"
          "........"
          "........"
          "........"
          "........"
          "........"
          "........"
          "........"
          "........"

      };


class RenderStringResource
: public ContextResource
{
    friend void renderString( ContextResourceCache* rc,
                              float w, float h,
                              float x, float y,
                              const std::string& str );
public:

    /**
      *
      * \sideeffect Current texture unit. Texture unit 0.
      */
    RenderStringResource()
    {
        width_ = 8u;
        height_ = 10u;
        layout_ = font_8x8_desc;


        cols_ = static_cast<GLsizei>( ceilf( sqrtf( static_cast<float>( layout_.size() )) ) );
        rows_ = (layout_.size()+cols_-1)/cols_;

        // organize glyphs into a gridded texture
        std::vector<GLubyte> texture( rows_*cols_*width_*height_*4 );
        GLsizei modulo = cols_*width_*4u;
        for(size_t k=0; k<font_8x8_desc.size(); k++) {
            GLsizei col = k % cols_;
            GLsizei row = k / cols_;
            for(int j=0; j<height_; j++) {
                for(int i=0; i<width_; i++) {
                    GLubyte texel[4];
                    switch( font_8x8_data[ width_*height_*k + width_*j + i] )
                    {
                    case '1':
                        texel[0] = 65u; texel[1] = 65u; texel[2] = 65u; texel[3] = 65u;
                        break;
                    case '2':
                        texel[0] = 200u; texel[1] = 200u; texel[2] = 200u; texel[3] = 200u;
                        break;
                    case '3':
                        texel[0] = 255u; texel[1] = 255u; texel[2] = 255u; texel[3] = 255u;
                        break;

                    default:
                        texel[0] = 0u; texel[1] = 0u; texel[2] = 0u; texel[3] = 0u;
                        break;
                    }
                    for(unsigned int l=0u; l<4u; l++) {
                        texture[ (height_*row + j)*modulo + (width_*col+ i)*4 + l] = texel[l];
                    }
                }
            }
        }
        glActiveTextureARB( GL_TEXTURE0_ARB );
        glGenTextures( 1, &tex_name_ );
        glBindTexture( GL_TEXTURE_2D, tex_name_ );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, cols_*width_, rows_*height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, &texture[0] );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        CHECK_GL;

        // create shader

/*        std::stringstream vs;

        vs << "#define COLS (" << cols_ << ")\n";
        vs << "#define ROWS (" << rows_ << ")\n";
        vs << "#define FW ("<< width_ << ")\n";
        vs << "#define WH ("<< height_ << ")\n";
        vs << "uniform int char;\n";
        vs << "void\n";
        vs << "main()\n";
        vs << "{\n";
        vs << "    ivec2 to = ivec2( FW*(char % COLS),\n";
        vs << "                      FH*(char / COLS) );\n";
        vs << "    gl_Position = gl_Vertex;\n";

        const char vs_src =
                "uniform int cols;\n"
                "uniform int rows;\n"
                "uniform
*/
    }

    ~RenderStringResource()
    {
        glDeleteTextures( 1, &tex_name_ );
    }

protected:

    void
    render( float w, float h,
            float x, float y,
            const std::string& str )
    {
        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        glLoadIdentity();


        glUseProgram( 0 );
        glViewport( 0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h) );

        glActiveTextureARB( GL_TEXTURE0_ARB );
        glBindTexture( GL_TEXTURE_2D, tex_name_ );
        glEnable( GL_TEXTURE_2D );

        glColor3f( 1.0f, 1.0f, 1.0f );


        glBegin( GL_QUADS );
        for(size_t i=0; i<str.size(); i++) {
            size_t k;
            for(k=0; k<layout_.size() && str[i] != layout_[k]; k++) {}


            float col = static_cast<float>(k % cols_);
            float row = static_cast<float>(k / cols_);

            glTexCoord2f( (col+0.0f)/(cols_), (row+0.0f)/(rows_) );
            glVertex2f( 2.0f*(x+0.0f)/w-1.0f, 1.0f-2.0f*(y+0.0f)/h );

            glTexCoord2f( (col+1.0f)/cols_, (row+0.0f)/rows_ );
            glVertex2f( 2.0f*(x+width_)/w-1.0f, 1.0f-2.0f*(y+0.0f)/h );

            glTexCoord2f( (col+1.0f)/cols_, (row+1.0f)/rows_ );
            glVertex2f( 2.0f*(x+width_)/w-1.0f, 1.0f-2.0f*(y+height_)/h );

            glTexCoord2f( (col+0.0f)/cols_, (row+1.0f)/rows_ );
            glVertex2f( 2.0f*(x+0)/w-1.0f, 1.0f-2.0f*(y+height_)/h );
            //std::cerr << ((x+8)/w) << "\n";
            x+= width_;
        }
        glEnd( );


        glDisable( GL_TEXTURE_2D );

        glMatrixMode( GL_PROJECTION );
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );
        glPopMatrix();

    }

    GLuint      tex_name_;
    GLsizei     width_;
    GLsizei     height_;
    GLsizei     cols_;
    GLsizei     rows_;
    std::string layout_;


};


inline void
renderString( ContextResourceCache* rc,
              float w, float h,
              float x, float y,
              const std::string& str )
{
    if( rc == NULL ) {
        return;
    }

    RenderStringResource* res =
            static_cast<RenderStringResource*>( rc->getResource( ContextResourceCache::RESOURCE_RENDERSTRING ) );
    if( res == NULL ) {
        res = new RenderStringResource();
        rc->setResource( ContextResourceCache::RESOURCE_RENDERSTRING, res );
    }
    res->render( w, h, x, y, str );

}




  } // of gl_utils
} // of siut

