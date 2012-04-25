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
