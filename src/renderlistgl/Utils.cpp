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
#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include <iomanip>
#include <string>
#include "Utils.hpp"

namespace tinia {
namespace renderlist {
namespace gl {

void
dumpGLError( const std::string& log, int line, GLenum error )
{
    while( error != GL_NO_ERROR ) {
        std::cerr << log;
        switch( error ) {
        case GL_INVALID_ENUM: std::cerr << "GL_INVALID_ENUM";break;
        case GL_INVALID_VALUE: std::cerr << "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: std::cerr << "GL_INVALID_OPERATION"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        case GL_OUT_OF_MEMORY: std::cerr << "GL_OUT_OF_MEMORY"; break;
        default: std::cerr << "GL error 0x" << std::hex << error << std::dec; break;
        }
        std::cerr << " at " << line << std::endl;
        error = glGetError();
    }
}

void
dumpGLFBOStatus( const std::string& log, int line, GLenum status )
{
    std::cerr << log;
    switch( status ) {
    case GL_FRAMEBUFFER_UNDEFINED: std::cerr << "GL_FRAMEBUFFER_UNDEFINED"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
    case GL_FRAMEBUFFER_UNSUPPORTED: std::cerr << "GL_UNSUPPORTED"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: std::cerr << "GL_INCOMPLETE_MULTISAMPLE"; break;
    default: std::cerr << "GL FBO status 0x" << std::hex << status << std::dec; break;
    }
    std::cerr << " at " << line << std::endl;
}


} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
