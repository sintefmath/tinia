#include "tinia/qtcontroller/moc/OpenGLServerGrabber.hpp"
#include <GL/glew.h>
#include <QImage>
#include <QBuffer>
#include <tinia/qtcontroller/impl/http_utils.hpp>

namespace tinia {
namespace qtcontroller {
namespace impl {

OpenGLServerGrabber::OpenGLServerGrabber(tinia::jobcontroller::Job* job,
                                         QObject *parent) :
    QObject(parent), m_glImageIsReady(false), m_job(job), m_buffer( NULL ), m_buffer_size(0), m_openglIsReady(false),
    m_width(500), m_height(500)
{
    connect(this, SIGNAL(glImageReady()), this,
            SLOT(wakeListeners()));

    connect(this, SIGNAL(getGLImage(uint,uint,QString)), this,
            SLOT(getImage(uint,uint,QString)));
}

OpenGLServerGrabber::~OpenGLServerGrabber()
{
    if(m_openglIsReady) {
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteRenderbuffers(1, &m_renderbufferRGBA);
        glDeleteRenderbuffers(1, &m_renderbufferDepth);
    }
}

void OpenGLServerGrabber::getImageAsText(QTextStream &os, unsigned int width, unsigned int height, QString key)
{
    m_mainMutex.lock();
    emit getGLImage(width, height, key);
    m_waitMutex.lock();
    while(!m_glImageIsReady) {
        m_waitCondition.wait(&m_waitMutex);
    }

    QImage img(m_buffer, width, height, QImage::Format_RGB888);

    // This is a temporary fix. The image is reflected through the horizontal
    // line y=height ((x, y) |--> (x, h-y) ).
    QTransform flipTransformation(1, 0,
                                  0, -1,
                                  0, height);
    img = img.transformed(flipTransformation);


    QBuffer qBuffer;

    img.save(&qBuffer, "png");
    os << httpHeader(getMimeType("file.txt"));
    QString str(QByteArray(qBuffer.data(), int(qBuffer.size())).toBase64());
    os << "\r\n"<<str;

    m_glImageIsReady = false;
    m_waitMutex.unlock();
    m_mainMutex.unlock();
}

void OpenGLServerGrabber::getImage(unsigned int width, unsigned int height, QString key)
{
    tinia::jobcontroller::OpenGLJob* openGLJob = static_cast<tinia::jobcontroller::OpenGLJob*>(m_job);
    if(!openGLJob) {
        throw std::invalid_argument("This is not an OpenGL job!");
    }
    if(!m_openglIsReady) {
        setupOpenGL();
    }

    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );

    if(m_width != width || m_height != height) {
        resize(width, height);
    }

    glViewport( 0, 0, width, height );



    openGLJob->renderFrame( "session", key.toStdString(), m_fbo, width, height );

    // make sure that buffer is large enough to hold raw image
    size_t req_buffer_size = width*height*3;
    if( (m_buffer == NULL) || (m_buffer_size < req_buffer_size) ) {
        if( m_buffer != NULL ) {
            delete m_buffer;
        }
        m_buffer_size = req_buffer_size;
        m_buffer = new unsigned char[m_buffer_size];
    }
    
     glPixelStorei( GL_PACK_ALIGNMENT, 1 );

     glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, m_buffer );



     glBindFramebuffer(GL_FRAMEBUFFER, 0);
     emit glImageReady();
}

void OpenGLServerGrabber::wakeListeners()
{
    m_waitMutex.lock();
    m_glImageIsReady = true;
    m_waitCondition.wakeAll();
    m_waitMutex.unlock();
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
    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
    glBindRenderbuffer( GL_RENDERBUFFER, m_renderbufferRGBA );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA, width, height );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_RENDERBUFFER,
                               m_renderbufferRGBA );

    glBindRenderbuffer( GL_RENDERBUFFER, m_renderbufferDepth );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER,
                               GL_DEPTH_ATTACHMENT,
                               GL_RENDERBUFFER,
                               m_renderbufferDepth );
    m_width = width;
    m_height = height;
}

} // namespace impl
} // namespace qtcontroller
} // namespace tinia
