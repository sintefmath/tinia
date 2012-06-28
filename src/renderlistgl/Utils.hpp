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

#pragma once
#include <GL/glew.h>
#include <iostream>
#include <iomanip>

namespace tinia {
namespace renderlist {
namespace gl {

void dumpGLError( const std::string& log, int line, GLenum error );
void dumpGLFBOStatus( const std::string& log, int line, GLenum status );

} // of namespace gl
} // of namespace renderlist
} // of namespace tinia


#define CHECK_GL do {                                                           \
    GLenum error = glGetError();                                                \
    if( error != GL_NO_ERROR ) { dumpGLError( log, __LINE__, error ); }        \
} while(0)

#define CHECK_FBO do { \
    GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );                             \
    if( status != GL_FRAMEBUFFER_COMPLETE ) { dumpGLFBOStatus( log, __LINE__, status ); }  \
} while(0)
