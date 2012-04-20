#pragma once
#include <GL/glew.h>
#include <iostream>
#include <iomanip>

namespace tinia {
namespace librenderlist {
namespace gl {

void dumpGLError( const std::string& log, int line, GLenum error );
void dumpGLFBOStatus( const std::string& log, int line, GLenum status );

} // of namespace gl
} // of namespace librenderlist
} // of namespace tinia


#define CHECK_GL do {                                                           \
    GLenum error = glGetError();                                                \
    if( error != GL_NO_ERROR ) { dumpGLError( log, __LINE__, error ); }        \
} while(0)

#define CHECK_FBO do { \
    GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );                             \
    if( status != GL_FRAMEBUFFER_COMPLETE ) { dumpGLFBOStatus( log, __LINE__, status ); }  \
} while(0)
