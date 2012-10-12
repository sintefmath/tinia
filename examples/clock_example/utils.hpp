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
#include <stdexcept>
#include <sstream>

namespace tinia {
namespace example {
/** Prints or throw gl_error class if an error is encountered.
 * If CHECK_GL is defined, will dump errors to stderr.
 * If DEBUG_GL is defined, will throw gl_error with what error was encountered.
 * \param fname the filename of where the function is called from. (__FILE__)
 * \param line the line number of where it was called from. (__LINE__)
 *
 * \example printGLError(__FILE__, __LINE__);
 */
    inline void printGLError(std::string fname, int line)
    {
        size_t count = 0;
        GLenum error = glGetError();
        if( error != GL_NO_ERROR ) {
            std::stringstream s;
        s << fname << '@' << line << ": OpenGL error: ";

            do {
                switch( error ) {
                case GL_INVALID_ENUM: s << "GL_INVALID_ENUM "; break;
                case GL_INVALID_VALUE: s << "GL_INVALID_VALUE "; break;
                case GL_INVALID_OPERATION: s << "GL_INVALID_OPERATION "; break;
                case GL_INVALID_FRAMEBUFFER_OPERATION: s << "GL_INVALID_FRAMEBUFFER_OPERATION "; break;
                case GL_OUT_OF_MEMORY: s << "GL_OUT_OF_MEMORY"; break;
                default:
                    s << "0x" << std::hex << error << std::dec << " "; break;
                }
                error = glGetError();
                ++count;
            } while( error != GL_NO_ERROR && count < 10 );

            throw std::runtime_error( s.str() );
        }
    }
}
}
#define CHECK_GL do { tinia::example::printGLError( __FILE__, __LINE__ ); } while(0)
