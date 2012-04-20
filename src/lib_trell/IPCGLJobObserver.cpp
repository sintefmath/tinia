#include <iostream>
#include <tinia/librenderlist/XMLWriter.hpp>
#include "tinia/trell/IPCGLJobObserver.hpp"


namespace Trell {


IPCGLJobObserver::IPCGLJobObserver(bool is_master)
    : IPCJobObserver( is_master ),
      m_openGLJob( NULL ),
      m_display( NULL ),
      m_context( NULL )
{
}

bool
IPCGLJobObserver::init( const std::string& xml )
{
   // Initialize this
   m_openGLJob = static_cast<jobobserver::OpenGLJob*>(m_job);
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

    bool ipcRetVal = IPCJobObserver::init(xml);
    return (ipcRetVal && m_openGLJob->initGL());

}

bool
IPCGLJobObserver::onGetRenderlist( size_t&             result_size,
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
    const librenderlist::DataBase* db = m_openGLJob->getRenderList( session, key );
    if( db == NULL ) {
        result_size = 0;
        return true;
    }

    std::string list = librenderlist::getUpdateXML( db,
                                                    librenderlist::ENCODING_JSON,
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


bool
IPCGLJobObserver::onGetSnapshot( char*               buffer,
                                 TrellPixelFormat    pixel_format,
                                 const size_t        width,
                                 const size_t        height,
                                 const std::string&  session,
                                 const std::string&  key )
{
  GLuint fbo_;
    glXMakeCurrent( m_display, m_pbuffer, m_context );

    if( key.empty() ) {
        std::cerr << "No key given." << std::endl;
        return false;
    }
    if( width < 1 || height < 1 ) {
        std::cerr << "Size must be at least 1x1 pixels" << std::endl;
        return false;
    }

    auto it = m_render_environments.find( key );
    if( it == m_render_environments.end() ) {
        std::cerr << "key '" << key << "' not defined, creating new environment." << std::endl;
        RenderEnvironment e;
        glGenFramebuffers( 1, &e.m_fbo );
        glGenRenderbuffers( 1, &e.m_renderbuffer_rgba );
        glGenRenderbuffers( 1, &e.m_renderbuffer_depth );
        m_render_environments[ key ] = e;
        it = m_render_environments.find( key );
        if( it == m_render_environments.end() ) {
            std::cerr << "Failed to re-find insert key." << std::endl;
            return false;
        }
    }

    glBindFramebuffer( GL_FRAMEBUFFER, it->second.m_fbo );
    fbo_ = it->second.m_fbo;
    if( it->second.m_width != width || it->second.m_height != height ) {

        std::cerr << "Resizing renderbuffers." << std::endl;

        glBindRenderbuffer( GL_RENDERBUFFER, it->second.m_renderbuffer_rgba );
        glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA, width, height );
        glBindRenderbuffer( GL_RENDERBUFFER, 0 );
        glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_RENDERBUFFER,
                                   it->second.m_renderbuffer_rgba );

        glBindRenderbuffer( GL_RENDERBUFFER, it->second.m_renderbuffer_depth );
        glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height );
        glBindRenderbuffer( GL_RENDERBUFFER, 0 );
        glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                                   GL_DEPTH_ATTACHMENT,
                                   GL_RENDERBUFFER,
                                   it->second.m_renderbuffer_depth );


        if( !checkFramebufferCompleteness() ) {
            std::cerr << "Incomplete framebuffer." << std::endl;
            it->second.m_width = 0;
            it->second.m_height = 0;
            return false;
        }
        else {
            it->second.m_width = width;
            it->second.m_height = height;
        }
    }
    glViewport( 0, 0, width, height );
    if( !checkForGLError() ) {
        std::cerr << "OpenGL error before rendering." << std::endl;
        return false;
    }
    //add fbo after key, before width
    if( !m_openGLJob->renderFrame( session, key, fbo_, width, height ) ) {
        std::cerr << "Rendering failed." << std::endl;
        return false;
    }
    if( !checkForGLError() ) {
        std::cerr << "OpenGL error during rendering." << std::endl;
        return false;
    }



    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    switch( pixel_format ) {
    case TRELL_PIXEL_FORMAT_BGR8:
        glReadPixels( 0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, buffer );
        break;
    default:
        std::cerr << "Unsupported pixel format." << std::endl;
        return false;
    }

/*
    for( size_t j=0; j<height; j++ ) {
        for( size_t i=0; i<width; i++) {
            buffer[ 3*(width*j + i) + 0 ] = i;
            buffer[ 3*(width*j + i) + 1 ] = j;
            buffer[ 3*(width*j + i) + 2 ] = 0;
        }
    }
*/

    return true;
}

void
IPCGLJobObserver::cleanup()
{
    IPCJobObserver::cleanup();

    if( m_context != NULL ) {
        glXDestroyContext( m_display, m_context );
    }
    if( m_display != NULL ) {
        XCloseDisplay( m_display );
    }
}

#define FOO(a) case a: std::cerr << (#a) << std::endl; break

bool
IPCGLJobObserver::checkFramebufferCompleteness() const
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
IPCGLJobObserver::checkForGLError() const
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

} // of namespace Trell

