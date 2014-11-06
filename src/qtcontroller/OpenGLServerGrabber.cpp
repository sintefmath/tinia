#include "tinia/qtcontroller/moc/OpenGLServerGrabber.hpp"
#include <GL/glew.h>
#include <QImage>
#include <QBuffer>
#include <tinia/qtcontroller/impl/http_utils.hpp>
#include "tinia/renderlist.hpp"

namespace tinia {
namespace qtcontroller {
namespace impl {


OpenGLServerGrabber::OpenGLServerGrabber( QObject *parent)
    : QObject(parent),
      m_buffer( NULL ),
      m_buffer_size(0),
      m_openglIsReady(false),
      m_width(500),
      m_height(500)
{
}


OpenGLServerGrabber::~OpenGLServerGrabber()
{
    if(m_openglIsReady) {
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteRenderbuffers(1, &m_renderbufferRGBA);
        glDeleteRenderbuffers(1, &m_renderbufferDepth);
    }
}


void
OpenGLServerGrabber::grabRGB( jobcontroller::OpenGLJob *job,
                              unsigned int width,
                              unsigned int height,
                              const std::string& key)
{
    if( !m_openglIsReady ) {
        setupOpenGL();
    }
    if(m_width != width || m_height != height) {
        resize(width, height);
    }

    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
    glViewport( 0, 0, width, height );
    job->renderFrame( "session", key, m_fbo, width, height );

    // QImage requires scanline size to be a multiple of 32 bits.
    size_t scanline_size = 4*((3*width+3)/4);
    glPixelStorei( GL_PACK_ALIGNMENT, 4 );

    // make sure that buffer is large enough to hold raw image
    size_t req_buffer_size = scanline_size*height*3;
    if( (m_buffer == NULL) || (m_buffer_size < req_buffer_size) ) {
        if( m_buffer != NULL ) {
            delete m_buffer;
        }
        m_buffer_size = req_buffer_size;
        m_buffer = new unsigned char[m_buffer_size];
    }

    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
    glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, m_buffer );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#if 0
        static int cntr=0;
        {
            char fname[1000];
            sprintf(fname, "/tmp/qtrgb_%05d.ppm", cntr);
            FILE *fp = fopen(fname, "w");
            fprintf(fp, "P6\n%d\n%d\n255\n", width, height);
            fwrite(m_buffer, 1, 3*width*height, fp);
            fclose(fp);
        }
        cntr++;
#endif
}


void
OpenGLServerGrabber::grabDepth( jobcontroller::OpenGLJob *job,
                                unsigned int width,
                                unsigned int height,
                                const std::string& key)
{
    if( !m_openglIsReady ) {
        setupOpenGL();
    }
    if(m_width != width || m_height != height) {
        resize(width, height);
    }

    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
    glViewport( 0, 0, width, height );
    // Should not be necessary to render, we will always have rendered for the RGB-grab before doing a depth-grab!
    // (On the other hand, in most (all?) cases, this second rendering will be discarded due to the RGB-rendering already being
    // in progress, right?)
    // job->renderFrame( "session", key, m_fbo, width, height );

    // QImage requires scanline size to be a multiple of 32 bits.
    size_t scanline_size = 4*width;
    glPixelStorei( GL_PACK_ALIGNMENT, 1 ); // 4 and 1 equally good, in this case?

    // make sure that buffer is large enough to hold raw image
    size_t req_buffer_size = scanline_size*height*4;
    if( (m_buffer == NULL) || (m_buffer_size < req_buffer_size) ) {
        if( m_buffer != NULL ) {
            delete m_buffer;
        }
        m_buffer_size = req_buffer_size;
        m_buffer = new unsigned char[m_buffer_size];
    }

    assert( sizeof(GL_FLOAT) == 4 );

    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
    glReadPixels( 0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, m_buffer );
    // Depth encoded as 24 bit fixed point values.
    for (size_t i=0; i<width*height; i++) {
        float value = ((float *)m_buffer)[i];
        for (size_t j=0; j<3; j++) {
            ((unsigned char *)m_buffer)[3*i+j] = (unsigned char)( floor(value*255.0) );
            value = 255.0*value - floor(value*255.0);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#if 0
    // Debug code for writing out the depth map to disk.
    static int cntr=0;
    {
        char fname[1000];
        sprintf(fname, "/tmp/qtdepth_%05d.ppm", cntr);
        FILE *fp = fopen(fname, "w");
        fprintf(fp, "P6\n%d\n%d\n255\n", width, height);
        fwrite(m_buffer, 1, 3*width*height, fp);
        fclose(fp);
    }
    cntr++;
#endif
}


void OpenGLServerGrabber::setupOpenGL()
{
    glewInit();
    glGenFramebuffers( 1, &m_fbo );
    glGenRenderbuffers( 1, &m_renderbufferRGBA );
    glGenRenderbuffers( 1, &m_renderbufferDepth );
    resize(m_width, m_height);
    m_openglIsReady = true;
}


void OpenGLServerGrabber::resize(unsigned int width, unsigned int height)
{
    // resize render buffers
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    glBindRenderbuffer( GL_RENDERBUFFER, m_renderbufferRGBA );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA, width, height );

    glBindRenderbuffer( GL_RENDERBUFFER, m_renderbufferDepth );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );

    // set up framebuffer
    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_RENDERBUFFER,
                               m_renderbufferRGBA );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                               GL_DEPTH_ATTACHMENT,
                               GL_RENDERBUFFER,
                               m_renderbufferDepth );

    // check for framebuffer completeness
    GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if( status != GL_FRAMEBUFFER_COMPLETE ) {
        std::cerr << "OpenGLServerGrabber::resize(width=" << width
                  << ",height=" << height << "): Incomplete framebuffer: ";
        switch( status ) {
        case GL_FRAMEBUFFER_UNDEFINED:
            std::cerr << "GL_FRAMEBUFFER_UNDEFINED";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            std::cerr << "GL_FRAMEBUFFER_UNSUPPORTED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
            break;
        default:
            std::cerr << "<unknown status>"; break;
        }
        std::cerr << std::endl;
    }
    
    m_width = width;
    m_height = height;
}


} // namespace impl
} // namespace qtcontroller
} // namespace tinia
