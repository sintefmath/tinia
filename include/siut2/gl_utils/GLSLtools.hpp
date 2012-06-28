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
//#define DEBUG_GL

#if defined (CHECK_GL) || defined (DEBUG_GL)
#define GL_TESTING
#include <sstream>
#include <iostream>
#include "siut2/io_utils/snarf.hpp"
#ifdef DEBUG_GL
#include <stdexcept>
#endif
#endif

#include "siut2/exceptions/backtrace_exception.hpp"

#include <iostream>
#include <vector>
#include <string>
#include "siut2/io_utils/snarf.hpp"

#define FBO_ERROR(a) case a: printThrowFBOError(where, #a); break
namespace siut2
{
  /** \brief Namespace with tools for compiling GLSL shaders, check FBOs for completeness,
   * printing GL-errors and classes that are thrown if DEBUG_GL is defined.
   * If CHECK_GL is defined, will dump messages to stderr, whereas if DEBUG_GL is defined, will throw them with the relevant error class.
   */
  namespace gl_utils
  {
   


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Error-classes                                                                                                       /////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Shader error class. 
     * Thrown if an error is encountered when compiling a shader. 
     *Contains the infolog about what went wrong, as well as the source code that threw the error.
     */
    class shader_error : public exceptions::backtrace_exception
    {      
    public:
      explicit shader_error(const std::string &what)
        : exceptions::backtrace_exception(what)
      {
	;
      }
      
 
      virtual ~shader_error() throw()
      {
	;
      }

    };


    /** GL error class, thrown when printGLError() encounters an error and DEBUG_GL is defined. */
    class gl_error : public exceptions::backtrace_exception
    {
    public:
      explicit gl_error(const std::string &what)
        : exceptions::backtrace_exception(what)
      {
	;
      }

      virtual ~gl_error() throw()
      {
	;
      }
    };

    /** FBO error class, thrown when checkFramebufferStatus() encounteres an error and DEBUG_GL is defined. */
    class fbo_error : public exceptions::backtrace_exception
    {
    public:
      explicit fbo_error(const std::string &what)
        :backtrace_exception(what)
      {
	;
      }

      virtual ~fbo_error() throw()
      {
	;
      }

    };


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Functions                                                                                                           /////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

            throw gl_error( s.str() );
        }
    }

#define CHECK_GL do { siut2::gl_utils::printGLError( __FILE__, __LINE__ ); } while(0)

    inline
    void linkProgram( GLuint program )
    {
      glLinkProgram( program );

      GLint linkstatus;
      glGetProgramiv( program, GL_LINK_STATUS, &linkstatus );
      if( linkstatus != GL_TRUE ) {

	std::string log;
	
        GLint logsize;
	glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logsize );
        if( logsize > 0 ) {
            std::vector<GLchar> infolog( logsize+1 );
            glGetProgramInfoLog( program, logsize, NULL, &infolog[0] );
            log = std::string( infolog.begin(), infolog.end() );
        }
        else {
	  log = "Empty log.";
        }
		std::cerr << log << std::endl;
	throw shader_error( "GLSL link failed:\n" + log );
      }
    }


    /** Compiles the shader of type from src, returns the shader id generated by OpenGL.
     * Compiles the shader from src to the type specified from type.
     * Uses printGLError to chek for errors creating the shader.
     * If DEBUG_GL is defined, will throw shader_error, containing error and source, if there is a problem compiling the shader.
     * If CHECK_GL is defined it will dump the error and source to stderr.
     * 
     * \param src source of the shader.
     * \param type type of shader to compile GL_VERTEX_SHADER, GL_FRAGMENT_SHADER etc.
     * \param return GLuint shader_id.
     */
    inline GLuint compileShader(const std::string &src, GLenum type, bool fail_on_warnings = false )
    {
      GLuint shader_id = glCreateShader(type);
      printGLError(__FILE__, __LINE__);

      const char *p = src.c_str();
      glShaderSource(shader_id, 1, &p, NULL);
      printGLError(__FILE__, __LINE__);
      glCompileShader(shader_id);
      printGLError(__FILE__, __LINE__);


      GLint llength;
      GLint status;

      glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &llength);
      glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
      if( (status != GL_TRUE) || (fail_on_warnings && (llength > 1) ) ) {
	
	std::string infol;
	if(llength > 1) {
	  GLchar* infolog = new GLchar[llength];
	  glGetShaderInfoLog(shader_id, llength, NULL, infolog);
	  infol = std::string( infolog );
	  delete[] infolog;
	}
	else {
	  infol = "no log.";
	}

	std::stringstream s;
	if( status != GL_TRUE ) {
	  s << "Compilation returned errors." << std::endl;
	}
	s << infol << std::endl;
	io_utils::dumpSourceWithLineNumbers( src, s );
	std::cerr << s.str() << std::endl;
	throw shader_error( s.str() );
	/*

#ifdef DEBUG_GL
	  std::string ret;
	  ret = std::string(infolog) + "\n" + src;
	  throw shader_error(ret);
#else
	  std::cerr << "infolog: "<< infolog;
	  std::cerr << "infolog ended" << std::endl << std::endl;
	  delete[] infolog;
#endif
	}

      if(status == GL_FALSE)
	{
#if defined (DEBUG_GL) || defined(CHECK_GL)
	  std::stringstream s;      
	  s << "shader didn't compile. in:" << __FILE__ << " at:" <<  __LINE__ << std::endl;
#ifdef DEBUG_GL
	  s << io_utils::addLineNumbers(src) << std::endl;
	  throw shader_error(s.str());
#else
	  io_utils::dumpSourceWithLineNumbers(src, std::cerr);
	  std::cerr << std::endl;
#endif
#endif
	  return GL_FALSE;
	*/
	return 0;
      }	      
      return shader_id;
    }

	inline void updateShader( GLuint shader_id, const std::string &src )
	{
		const char *p = src.c_str();
		glShaderSource(shader_id, 1, &p, NULL);
		printGLError(__FILE__, __LINE__);
		glCompileShader(shader_id);
		printGLError(__FILE__, __LINE__);


		GLint llength;
		GLint status;

		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &llength);
		glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
		if( (status != GL_TRUE) || llength > 1 ) {

			std::string infol;
			if(llength > 1) {
				GLchar* infolog = new GLchar[llength];
				glGetShaderInfoLog(shader_id, llength, NULL, infolog);
				infol = std::string( infolog );
				delete[] infolog;
			}
			else {
				infol = "no log.";
			}

			std::stringstream s;
			if( status != GL_TRUE ) {
				s << "Compilation returned errors." << std::endl;
			}
			s << infol << std::endl;
			io_utils::dumpSourceWithLineNumbers( src, s );
			std::cerr << s.str() << std::endl;
			
		}
		
	}

    /** Method used by checkFramebufferStatus() to either throw or print if there is an error with the framebuffer.
     * Not really meant to be used elsewhere, but hey...
     * If DEBUG_GL is defined, will throw fbo_error with what has gone wrong.
     * If CHECK_GL is defined, will dump the error message to stderr.
     * \param where where checkFramebufferStatus() was called from
     * \param what what has gone wrong.
     */
    inline void printThrowFBOError(const std::string &where, const std::string &what)
    {
#if defined (DEBUG_GL) || defined(CHECK_GL)
      std::string ret("FBO Error in" + where + "\n" + "Problem is:\t" + what);
      std::cerr << ret << std::endl;
      throw new fbo_error(ret);
#endif
    }

    /** Method used to check completeness of a framebuffer, fbo.
     * Checks if there is any problems with the completeness of the framebuffer, 
     * If a problem is encountered it calls printThrowFBOError() with the error that was encountered, and where it was called from.
     *
     * \param where where checkFramebufferStatus was called from. (__FILE__ + __LINE__)
     *
     */
    inline void checkFramebufferStatus(const std::string &where)
    {
#ifdef GL_ARB_framebuffer_object
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if( status != GL_FRAMEBUFFER_COMPLETE ) {
			switch(status)
			{
				//	  FBO_ERROR(GL_FRAMEBUFFER_COMPLETE);
				FBO_ERROR(GL_FRAMEBUFFER_UNDEFINED);
				FBO_ERROR(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
				FBO_ERROR(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
				FBO_ERROR(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
				FBO_ERROR(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
				FBO_ERROR(GL_FRAMEBUFFER_UNSUPPORTED);
				FBO_ERROR(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
			default:
				{
					std::stringstream s;
					s << "unknown fbo error " << status << ".\n";
					printThrowFBOError(where, s.str() );
				}
			}
		}
#else //NVidia driver for XP only exposes EXT-versions 2009-09-18
		GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if( status != GL_FRAMEBUFFER_COMPLETE_EXT ) {
			switch(status)
			{
				//	  FBO_ERROR(GL_FRAMEBUFFER_COMPLETE);
				//FBO_ERROR(GL_FRAMEBUFFER_UNDEFINED);
				FBO_ERROR(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT);
				FBO_ERROR(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT);
				FBO_ERROR(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT);
				FBO_ERROR(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT);
				FBO_ERROR(GL_FRAMEBUFFER_UNSUPPORTED_EXT);
				FBO_ERROR(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT);
			default:
				{
					std::stringstream s;
					s << "unknown fbo error " << status << ".\n";
					printThrowFBOError(where, s.str() );
				}
			}
		}
#endif
    }



#define FBO_ERROR_HELPER( a ) case (a): msg << (#a); break;
inline
void
checkFBO( const std::string& file, int line )
{
#ifdef GL_ARB_framebuffer_object
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( status != GL_FRAMEBUFFER_COMPLETE ) {
        std::stringstream msg;
        msg << "Incomplete FBO ";
        switch( status ) {
            FBO_ERROR_HELPER(GL_FRAMEBUFFER_UNDEFINED);
            FBO_ERROR_HELPER(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
            FBO_ERROR_HELPER(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
            FBO_ERROR_HELPER(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
            FBO_ERROR_HELPER(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
            FBO_ERROR_HELPER(GL_FRAMEBUFFER_UNSUPPORTED);
            FBO_ERROR_HELPER(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
        default:
            msg << "unknown error (" << status << ")";
        }
        msg << " (" << file << "@" << line << ")";
        std::cerr << "ERROR: " << msg.str() << std::endl;
        throw exceptions::backtrace_exception( msg.str() );
    }
#else //NVidia driver for XP only exposes EXT-versions 2009-09-18
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
    if( status != GL_FRAMEBUFFER_COMPLETE_EXT ) {
        std::stringstream msg;
        msg << "Incomplete FBO ";
        switch( status ) {
            FBO_ERROR_HELPER(GL_FRAMEBUFFER_UNDEFINED_EXT);
            FBO_ERROR_HELPER(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT);
            FBO_ERROR_HELPER(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT);
            FBO_ERROR_HELPER(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT);
            FBO_ERROR_HELPER(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT);
            FBO_ERROR_HELPER(GL_FRAMEBUFFER_UNSUPPORTED_EXT);
            FBO_ERROR_HELPER(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT);
        default:
            msg << "unknown error (" << status << ")";
        }
        msg << " (" << file << "@" << line << ")";
        std::cerr << "ERROR: " << msg.str() << std::endl;
        throw exceptions::backtrace_exception( msg.str() );
    }
#endif
}
#undef FBO_ERROR_HELPER

#define CHECK_FBO do { siut2::gl_utils::checkFBO( __FILE__, __LINE__ ); } while(0)


    inline
    GLint
    _getVaryingLocation( GLuint program, const char* name, const std::string& file, int line )
    {
      GLint loc = glGetVaryingLocationNV( program, name );
      if( loc == -1 ) {
	std::stringstream out;
	out << file << '@' << line<< ": failed to get location of varying \"" << name << "\".";
        throw exceptions::backtrace_exception( out.str() );
      }
      return loc;
    }

#define getVaryingLocation(program,name) (siut::gl_utils::_getVaryingLocation((program),(name),__FILE__, __LINE__))


  }//end namespace GL
}

