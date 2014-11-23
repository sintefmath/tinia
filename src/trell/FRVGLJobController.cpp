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

#include <cstdlib>      // getenv
#include <sstream>
#include <tinia/renderlist/XMLWriter.hpp>
#include "tinia/trell/FRVGLJobController.hpp"

#include <GL/glut.h>

namespace {

static const std::string package = "FRVGLJobController";

struct GLDebugLogWrapperData
{
    void  (*m_logger_callback)( void* logger_data, int level, const char* who, const char* msg, ... );
    void*   m_logger_data;
};


#ifdef GLEW_khr_DEBUG // make sure the glew version is new enough
static
void
GLDebugLogWrapper( GLenum source,
                   GLenum type,
                   GLuint id,
                   GLenum severity,
                   GLsizei length,
                   const GLchar* message,
                   void* data )
{

    const char* source_str = "???";
    switch( source ) {
    case GL_DEBUG_SOURCE_API:             source_str = "api"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   source_str = "wsy"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: source_str = "cmp"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     source_str = "3py"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     source_str = "app"; break;
    case GL_DEBUG_SOURCE_OTHER:           source_str = "oth"; break;
    default: break;
    }

    const char* type_str = "???";
    switch( type ) {
    case GL_DEBUG_TYPE_ERROR:               type_str = "error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_str = "deprecated"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  type_str = "undef"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         type_str = "portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         type_str = "performance"; break;
    case GL_DEBUG_TYPE_OTHER:               type_str = "other"; break;
    default: break;
    }

    int level = 2;
    switch( severity ) {
    case GL_DEBUG_SEVERITY_HIGH:   level = 0; break;
    case GL_DEBUG_SEVERITY_MEDIUM: level = 1; break;
    default: break;
    }

    GLDebugLogWrapperData* d = reinterpret_cast<GLDebugLogWrapperData*>( data );
    d->m_logger_callback( d->m_logger_data,
                          level,
                          "OpenGL",
                          "src=%s;type=%s: %s",
                          source_str,
                          type_str,
                          message );
}
#endif

} // of anonymous namespace


namespace tinia {
namespace trell {


FRVGLJobController::FRVGLJobController(bool is_master)
    : m_openGLJob( NULL )
{
}

void FRVGLJobController::setJob( jobcontroller::Job* job )
{
    //no need to have it saved as anything else afaik
    m_openGLJob = static_cast<jobcontroller::OpenGLJob*>(job);
}

bool
FRVGLJobController::init()
{

    
    // --- set up OpenGL context ------------------------------------------------

    //set up Freeglut window here
    //is init called after run? If so I could use the actual values, but not really worth wile..
    int argc = 1;
    char* argv = "FRVGLJobController";
    glutInit(&argc, &argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize (1000, 1000);
    glutInitWindowPosition (1000, 100);
    glutCreateWindow ("FRViewJobDebugWindow");    


    
    // --- OpenGL context created, init glew etc. ------------------------------
    glewInit();

    if( m_logger_callback != NULL ) {
        GLint major, minor;
        glGetIntegerv( GL_MAJOR_VERSION, &major );
        glGetIntegerv( GL_MINOR_VERSION, &minor );
        
        m_logger_callback( m_logger_data, 2, package.c_str(),
                           "OpenGL %d.%d (%s, %s, %s).",
                           major, minor,
                           glGetString( GL_RENDERER ),
                           glGetString( GL_VERSION ),
                           glGetString( GL_VENDOR ) );
    }
    glGetIntegerv( GL_MAX_INTEGER_SAMPLES, &m_max_samples );


        if( glewIsSupported( "GL_KHR_debug" ) ) {
            GLDebugLogWrapperData* data = new GLDebugLogWrapperData;
            data->m_logger_callback = m_logger_callback;
            data->m_logger_data = m_logger_data;
            
            glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
            glDebugMessageCallback( GLDebugLogWrapper, data );
            glDebugMessageControl( GL_DONT_CARE,
                                   GL_DONT_CARE,
                                   GL_DEBUG_SEVERITY_NOTIFICATION,
                                   0, NULL, GL_TRUE );
            if( m_logger_callback != NULL ) {
                m_logger_callback( m_logger_data, 2, package.c_str(),
                                   "Enabled OpenGL debugging." );
            }
        }
        else {
            if( m_logger_callback != NULL ) {
                m_logger_callback( m_logger_data, 2, package.c_str(),
                                   "GL_KHR_debug not supported." );
            }
        }
   
    

    return m_openGLJob->initGL();

}


bool
FRVGLJobController::onGetSnapshot( char*               buffer,
                                   const size_t        width,
                                   const size_t        height,
                                   const std::string&  session,
                                   const std::string&  key )
{
    // bind context
    if( !m_context.bindContext() ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Failed to bind OpenGL context." );
        }
        return false;
    }
    // check if we have a key (and we should check if this corrensponds to a viewer)
    if( key.empty() ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "No key given." );
        }
        return false;
    }
    // sane size
    if( width < 1 || height < 1 ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Size must be at least 1x1 pixels." );
        }
        return false;
    }
    
    GLsizei samples = std::min( std::max( 0,
                                          (m_max_samples*m_quality+127)/255),
                                m_max_samples );
    

    // --- get render targets --------------------------------------------------    
    RenderEnvironment* env_render = getRenderEnvironment( width, height,
                                                          samples );
    if( env_render == NULL ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Failed to get rendering environment" );
        }
        return false;
    }
    RenderEnvironment* env_copy = env_render;
    if( env_render->m_samples > 1 ) {
        env_copy = getRenderEnvironment( width, height, 1 );
        if( env_copy == NULL ) {
            if( m_logger_callback != NULL ) {
                m_logger_callback( m_logger_data, 0, package.c_str(),
                                   "Failed to get de-msaa environment" );
            }
            return false;
        }
    }

    // --- render --------------------------------------------------------------
    glBindFramebuffer( GL_FRAMEBUFFER, env_render->m_fbo );
    glViewport( 0, 0, width, height );

    if( !checkForGLError() ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Got OpenGL error before Job's rendering code." );
        }
        return false;
    }

    if( env_render->m_samples > 1 ) {
        glEnable( GL_MULTISAMPLE );
    }
    bool res = m_openGLJob->renderFrame( session, key, env_render->m_fbo, width, height );
    if( env_render->m_samples > 1 ) {
        glDisable( GL_MULTISAMPLE );
    }
    
    if( res == false ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Job's rendering code failed" );
        }
    }
    if( !checkForGLError() ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Got OpenGL error in Job's rendering code." );
        }
        return false;
    }

    // --- if multisample, blit to non-multisample before reading --------------
    if( env_render != env_copy ) {
        glBindFramebuffer( GL_DRAW_FRAMEBUFFER, env_copy->m_fbo );
        glBindFramebuffer( GL_READ_FRAMEBUFFER, env_render->m_fbo );
        glBlitFramebuffer( 0, 0, width, height,
                           0, 0, width, height,
                           GL_COLOR_BUFFER_BIT,
                           GL_NEAREST );
    }
    
    // --- read pixels ---------------------------------------------------------
    glBindFramebuffer( GL_FRAMEBUFFER, env_copy->m_fbo );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    switch( pixel_format ) {
    case TRELL_PIXEL_FORMAT_RGB_JPG_VERSION: // @@@
    case TRELL_PIXEL_FORMAT_RGB:
        glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer );
        break;
    case TRELL_PIXEL_FORMAT_RGB_CUSTOM_DEPTH:
    {
        unsigned char *buffer_pos = (unsigned char *)buffer;
        glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer_pos );
        buffer_pos += 4*((width*height*3 + 3)/4); // As long as GL_PACK_ALIGNMENT is set to 1 above, this should be ok. (I.e., no padding for single scan lines.)
        // NB! We read four bytes per pixel, then convert to three, meaning that the buffer must be large enough for four!!!
        glReadPixels( 0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, buffer_pos );
        // Depth encoded as 24 bit fixed point values.
        for (size_t i=0; i<width*height; i++) {
            float value = (float)( ((GLfloat *)buffer_pos)[i] );
            for (size_t j=0; j<3; j++) {
                (buffer_pos)[3*i+j] = (unsigned char)( floor(value*255.0) );
                value = 255.0*value - floor(value*255.0);
            }
        }

       break;
    }
    default:
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Unsupported pixel format." );
        }
        return false;
    }

    return true;
}

void
FRVGLJobController::cleanup()
{
    //nuke window and such
    //or just don't do anything..
}



bool
FRVGLJobController::checkFramebufferCompleteness() const
{
    GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if( status == GL_FRAMEBUFFER_COMPLETE ) {
        return true;
    }
    if( m_logger_callback != NULL ) {
        const char* error = NULL;
        switch( status ) {
        case GL_FRAMEBUFFER_UNDEFINED:
            error = "GL_FRAMEBUFFER_UNDEFINED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            error = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            error = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            error = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            error = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            error = "GL_FRAMEBUFFER_UNSUPPORTED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            error = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            error = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
            break;
        default:
            break;
        }
        if( error != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Incomplete framebuffer: %s.", error );
        }
        else {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Incomplete framebuffer: %x", status );
        }
    }
    return false;
}

bool
FRVGLJobController::checkForGLError() const
{
    GLenum status = glGetError();
    if( status == GL_NO_ERROR ) {
        return true;
    }
    while( status != GL_NO_ERROR ) {
        if( m_logger_callback != NULL ) {
            const char* error = NULL;
            switch( status ) {
            case GL_INVALID_ENUM:
                error = "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "GL_INVALID_OPERATION";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error = "GL_OUT_OF_MEMORY";
                break;
            default:
                break;
            }
            if( error != NULL ) {
                m_logger_callback( m_logger_data, 0, package.c_str(),
                                   "OpenGL error: %s.", error );
                
            }
            else {
                m_logger_callback( m_logger_data, 0, package.c_str(),
                                   "OpenGL error: %x", status );
            }
            status = glGetError();
        }
    }
    return false;
}

} // of namespace trell
} // of namespace tinia

