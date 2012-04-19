#pragma once
#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glxext.h>
#include <unordered_map>
#include "jobobserver/OpenGLJob.hpp"
#include "IPCJobObserver.hpp"

namespace Trell {

class IPCGLJobObserver : public IPCJobObserver
{
public:
    IPCGLJobObserver( bool is_master = false );

protected:
    /** \copydoc MessageBox::init */
    virtual
    bool
    init( const std::string& xml );


    /** \copydoc MessageBox::cleanup */
    virtual
    void
    cleanup();

    virtual
    bool
    onGetSnapshot( char*               buffer,
                   TrellPixelFormat    pixel_format,
                   const size_t        width,
                   const size_t        height,
                   const std::string&  session,
                   const std::string&  key );

    virtual
    bool
    onGetRenderlist( size_t&             result_size,
                     char*               buffer,
                     const size_t        buffer_size,
                     const std::string&  session,
                     const std::string&  key,
                     const std::string&  timestamp );



private:
    jobobserver::OpenGLJob *m_openGLJob;
    struct RenderEnvironment {
        GLuint                                          m_fbo;
        GLuint                                          m_renderbuffer_rgba;
        GLuint                                          m_renderbuffer_depth;
        GLsizei                                         m_width;
        GLsizei                                         m_height;
    };


    Display*                                            m_display;
    GLXContext                                          m_context;


    GLXPbuffer                                          m_pbuffer;

    std::unordered_map<std::string, RenderEnvironment>  m_render_environments;

    bool
    checkFramebufferCompleteness() const;

    bool
    checkForGLError() const;

};


} // of namespace Trell
