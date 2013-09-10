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

#include <iostream>
#include <tinia/renderlist/XMLWriter.hpp>
#include "tinia/trell/IPCGLJobController.hpp"

namespace tinia {
namespace trell {


IPCGLJobController::IPCGLJobController(bool is_master)
    : IPCJobController( is_master ),
      m_openGLJob( NULL ),
      m_quality( 0 ),
      m_display( NULL ),
      m_context( NULL )
{
}


static void debugLogger( GLenum source,
                                  GLenum type,
                                  GLuint id,
                                  GLenum severity,
                                  GLsizei length,
                                  const GLchar* message,
                                  void* data )
{
    std::cerr << "OpenGL debug [src=";
    switch( source ) {
    case GL_DEBUG_SOURCE_API:               std::cerr << "api"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     std::cerr << "wsy"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:   std::cerr << "cmp"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:       std::cerr << "3py"; break;
    case GL_DEBUG_SOURCE_APPLICATION:       std::cerr << "app"; break;
    case GL_DEBUG_SOURCE_OTHER:             std::cerr << "oth"; break;
    default:                                std::cerr << "???"; break;
    }

    std::cerr << ", type=";
    switch( type ) {
    case GL_DEBUG_TYPE_ERROR:               std::cerr << "error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr <<  "deprecated"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr <<  "undef"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cerr <<  "portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr <<  "performance"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cerr <<  "other"; break;
    default:                                std::cerr << "???"; break;
    }

    std::cerr << ", severity=";
    switch( severity ) {
    case GL_DEBUG_SEVERITY_HIGH:            std::cerr <<  "high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:          std::cerr <<  "medium"; break;
    case GL_DEBUG_SEVERITY_LOW:             std::cerr <<  "low"; break;
    default:                                std::cerr << "???"; break;
    }

    std::cerr << "] " << message << std::endl;
}

void
IPCGLJobController::setQuality( int quality )
{
    m_quality = std::max( 0, std::min( 255, quality ) );
}

bool
IPCGLJobController::init()
{
   // Initialize this
   m_openGLJob = static_cast<jobcontroller::OpenGLJob*>(m_job);
    // FIXME: This is the mapping to which GPU the thread will work against, so
    // using something more clever than a static string is appropriate.
    static std::string display = ":0.0";

    m_display = XOpenDisplay( display.c_str() );
    if( m_display == NULL ) {
        std::cerr << "Failed to open display '" << display << "'." << std::endl;
        return false;
    }

    int framebufffer_attribs[] = {
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        None
    };
    int framebuffer_configs_N;
    GLXFBConfig* framebuffer_configs = glXChooseFBConfig( m_display,
                                                          DefaultScreen( m_display),
                                                          framebufffer_attribs,
                                                          &framebuffer_configs_N );
    if( (framebuffer_configs == NULL) || (framebuffer_configs_N == 0 ) ) {
        std::cerr << "Failed to find suitable framebuffer configs." << std::endl;
        return false;
    }

    int pbuffer_attribs[] = {
        GLX_PBUFFER_WIDTH, 500,
        GLX_PBUFFER_HEIGHT, 500,
        GLX_LARGEST_PBUFFER, GL_FALSE,
        GLX_PRESERVED_CONTENTS, GL_TRUE,
        None
    };
    m_pbuffer = glXCreatePbuffer( m_display,
                                  framebuffer_configs[0],
                                  pbuffer_attribs );
    // FIXME: How to check for errors? Aren't these sent as events?
    m_context = glXCreateNewContext( m_display,
                                     framebuffer_configs[0],
                                     GLX_RGBA_TYPE,
                                     NULL,
                                     GL_TRUE );
    if( m_context == NULL ) {
        std::cerr << "Failed to create context." << std::endl;
        XFree( framebuffer_configs );
        return false;
    }
    glXMakeCurrent( m_display, m_pbuffer, m_context );
    glewInit();
    
    
    GLint major, minor;
    glGetIntegerv( GL_MAJOR_VERSION, &major );
    glGetIntegerv( GL_MINOR_VERSION, &minor );
    std::cerr << "OpenGL " << major << "." << minor << "\n";

    glGetIntegerv( GL_MAX_INTEGER_SAMPLES, &m_samples );
    std::cerr << "Max samples = " << m_samples << "\n";

    bool debug = true;
    if( debug ) {
        if( glewIsSupported( "GL_KHR_debug" ) ) {
            glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
            glDebugMessageCallback( debugLogger, NULL );
            glDebugMessageControl( GL_DONT_CARE,
                                   GL_DONT_CARE,
                                   GL_DEBUG_SEVERITY_NOTIFICATION,
                                   0, NULL, GL_TRUE );
            std::cerr << "Enabled OpenGL debugging.\n";
        }
        else {
            std::cerr << "OpenGL debugging not supported.\n";
        }
    }
    
        
    
    bool ipcRetVal = IPCJobController::init();
    return (ipcRetVal && m_openGLJob->initGL());

}

bool
IPCGLJobController::onGetRenderlist( size_t&             result_size,
                                   char*               result_buffer,
                                   const size_t        result_buffer_size,
                                   const std::string&  session,
                                   const std::string&  key,
                                   const std::string&  timestamp )
{
    // FIXME: Send this as an uint all the way through.
    unsigned int client_revision = 0;
    try {
        client_revision = boost::lexical_cast<unsigned int>( timestamp );
    }
    catch( boost::bad_lexical_cast& e ) {
        std::cerr << "Failed to parse revision number '" << client_revision << "'.\n";
    }
    if( m_openGLJob == NULL ) {
        return false;
    }
    const renderlist::DataBase* db = m_openGLJob->getRenderList( session, key );
    if( db == NULL ) {
        result_size = 0;
        return true;
    }

    std::string list = renderlist::getUpdateXML( db,
                                                    renderlist::ENCODING_JSON,
                                                    client_revision );
    if( list.length()+1 < result_buffer_size ) {
        strcpy(result_buffer, list.c_str());
        result_size = list.size();
        return true;
    }
    else {
        std::cerr << "Insufficent buffer size for renderlist.\n";
        return false;
    }
}


void
IPCGLJobController::dumpEnvironmentList()
{
    for( auto it = m_environments.begin(); it!=m_environments.end(); ++it ) {
        std::cerr << "["
                  << (*it)->m_width
                  << "x"
                  << (*it)->m_height
                  << "x"
                  << (*it)->m_samples
                  << "] ";
    }
    std::cerr << "\n";
}


IPCGLJobController::RenderEnvironment*
IPCGLJobController::getRenderEnvironment( GLsizei width, GLsizei height, GLsizei samples )
{
    // --- bump width and height up to the nearest power-of-two ----------------
    GLsizei w2 = width - 1;
    w2 = w2 | (w2>>1);
    w2 = w2 | (w2>>2);
    w2 = w2 | (w2>>4);
    w2 = w2 | (w2>>8);
    w2 = w2 | (w2>>16);
    w2 = w2 + 1;
    GLsizei h2 = height - 1;
    h2 = h2 | (h2>>1);
    h2 = h2 | (h2>>2);
    h2 = h2 | (h2>>4);
    h2 = h2 | (h2>>8);
    h2 = h2 | (h2>>16);
    h2 = h2 + 1;

    // --- Check if we already have a suitable environment ---------------------    
    for( auto it = m_environments.begin(); it!=m_environments.end(); ++it ) {
        if( ((*it)->m_width == w2 ) && ((*it)->m_height == h2) && ((*it)->m_samples == samples ) ) {
            // move to front
            if( it != m_environments.begin() ) {
                m_environments.splice( m_environments.begin(),
                                       m_environments,
                                       it,
                                       std::next( it ) );
                dumpEnvironmentList();
            }
            
            return *it;
        }
    }
    
    // --- Delete the least recently used environments -------------------------
    while( m_environments.size() > 10 ) {
        glDeleteFramebuffers( 1, &m_environments.back()->m_fbo );
        glDeleteRenderbuffers( 1, &m_environments.back()->m_renderbuffer_rgba );
        glDeleteRenderbuffers( 1, &m_environments.back()->m_renderbuffer_depth );
        delete m_environments.back();
        m_environments.pop_back();
    }
    if( !checkForGLError() ) {
        std::cerr << __LINE__ << "\n";
    }
    

    RenderEnvironment* e = new RenderEnvironment;
    
    glGenFramebuffers( 1, &e->m_fbo );
    glGenRenderbuffers( 1, &e->m_renderbuffer_rgba );
    glGenRenderbuffers( 1, &e->m_renderbuffer_depth );

    if( samples > 1 ) {
        glBindRenderbuffer( GL_RENDERBUFFER, e->m_renderbuffer_rgba );
        glRenderbufferStorageMultisample( GL_RENDERBUFFER, samples, GL_RGBA, w2, h2 );
        glBindRenderbuffer( GL_RENDERBUFFER, e->m_renderbuffer_depth );
        glRenderbufferStorageMultisample( GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT, w2, h2 );
    }
    else {
        glBindRenderbuffer( GL_RENDERBUFFER, e->m_renderbuffer_rgba );
        glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA, w2, h2 );
        glBindRenderbuffer( GL_RENDERBUFFER, e->m_renderbuffer_depth );
        glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w2, h2 );
    }
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );

    glBindFramebuffer( GL_FRAMEBUFFER, e->m_fbo );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, e->m_renderbuffer_rgba );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, e->m_renderbuffer_depth );

    if(!checkFramebufferCompleteness() ) {
        std::cerr << "Failed to create framebuffer ["
                  << w2 << "x" << h2 << "x" << samples << "]\n";
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glDeleteFramebuffers( 1, &e->m_fbo );
        glDeleteRenderbuffers( 1, &e->m_renderbuffer_rgba );
        glDeleteRenderbuffers( 1, &e->m_renderbuffer_depth );
        delete e;
        return NULL;
    }
    else {
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        e->m_width = w2;
        e->m_height = h2;
        e->m_samples = samples;
        std::cerr << "Created framebuffer ["
                  << w2 << "x" << h2 << "x" << samples << "]\n";
        m_environments.push_front( e );
        dumpEnvironmentList();
        return e;
    }
    
}


bool
IPCGLJobController::onGetSnapshot( char*               buffer,
                                 TrellPixelFormat    pixel_format,
                                 const size_t        width,
                                 const size_t        height,
                                 const std::string&  session,
                                 const std::string&  key )
{
    glXMakeCurrent( m_display, m_pbuffer, m_context );
    if( key.empty() ) {
        std::cerr << "No key given." << std::endl; return false;
    }
    if( width < 1 || height < 1 ) {
        std::cerr << "Size must be at least 1x1 pixels" << std::endl; return false;
    }
    
    GLsizei samples = std::min( std::max( 0,
                                          (m_samples*m_quality+127)/255),
                                m_samples );
    

    // --- get render targets --------------------------------------------------    
    RenderEnvironment* env_render = getRenderEnvironment( width, height,
                                                          samples );
    if( env_render == NULL ) {
        std::cerr << "Error at " << __LINE__ << "\n"; return false;
    }
    RenderEnvironment* env_copy = env_render;
    if( env_render->m_samples > 1 ) {
        env_copy = getRenderEnvironment( width, height, 1 );
        if( env_copy == NULL ) {
            std::cerr << "Error at " << __LINE__ << "\n"; return false;
        }
    }

    // --- render --------------------------------------------------------------
    glBindFramebuffer( GL_FRAMEBUFFER, env_render->m_fbo );
    glViewport( 0, 0, width, height );

    if( env_render->m_samples > 1 ) {
        glEnable( GL_MULTISAMPLE );
    }
    bool res = m_openGLJob->renderFrame( session, key, env_render->m_fbo, width, height );
    if( env_render->m_samples > 1 ) {
        glDisable( GL_MULTISAMPLE );
    }
    
    if( res == false ) {
        std::cerr << "Rendering failed." << std::endl; return false;
    }

    if( !checkForGLError() ) {
        std::cerr << "OpenGL error during rendering." << std::endl; return false;
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
    case TRELL_PIXEL_FORMAT_BGR8:
        glReadPixels( 0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, buffer );
        break;
    default:
        std::cerr << "Unsupported pixel format." << std::endl;
        return false;
    }

    return true;
}

void
IPCGLJobController::cleanup()
{
    IPCJobController::cleanup();

    if( m_context != NULL ) {
        glXDestroyContext( m_display, m_context );
    }
    if( m_display != NULL ) {
        XCloseDisplay( m_display );
    }
}

#define FOO(a) case a: std::cerr << (#a) << std::endl; break

bool
IPCGLJobController::checkFramebufferCompleteness() const
{
    GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    switch( status ) {
    case GL_FRAMEBUFFER_COMPLETE:
        return true;
        break;
        FOO( GL_FRAMEBUFFER_UNDEFINED );
        FOO( GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT );
        FOO( GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT );
        FOO( GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER );
        FOO( GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER );
        FOO( GL_FRAMEBUFFER_UNSUPPORTED );
        FOO( GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE );
        FOO( GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS );
    default:
        std::cerr << "unrecognized FBO error " << std::hex << status << std::endl;
        break;
    }
    return false;
}

bool
IPCGLJobController::checkForGLError() const
{
    GLenum status = glGetError();
    if( status == GL_NO_ERROR ) {
        return true;
    }
    while( status != GL_NO_ERROR ) {
        switch( status ) {
        FOO( GL_INVALID_ENUM );
        FOO( GL_INVALID_VALUE );
        FOO( GL_INVALID_OPERATION );
        FOO( GL_INVALID_FRAMEBUFFER_OPERATION );
        FOO( GL_OUT_OF_MEMORY );
        default:
            std::cerr << "unrecognized GL error " << std::hex << status << std::endl;
            break;
        }
        status = glGetError();
    }
    return false;

}
#undef FOO

} // of namespace trell
} // of namespace tinia

