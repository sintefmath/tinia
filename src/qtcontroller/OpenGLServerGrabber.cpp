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
QObject(parent), m_glImageIsReady(false), m_job(job), m_openglIsReady(false),
    m_width(500), m_height(500)
{
    connect(this, SIGNAL(glImageReady()), this,
            SLOT(wakeListeners()));
    connect(this, SIGNAL(getGLImage(uint,uint,QString)), this,
            SLOT(getImage(uint,uint,QString)));
    connect(this, SIGNAL(getGLDepthBuffer(uint,uint,QString)), this,
            SLOT(getDepthBuffer(uint,uint,QString)));
}


OpenGLServerGrabber::~OpenGLServerGrabber()
{
    if(m_openglIsReady) {
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteRenderbuffers(1, &m_renderbufferRGBA);
        glDeleteRenderbuffers(1, &m_renderbufferDepth);
    }
}


void OpenGLServerGrabber::getImageAsTextCommon(QString &str, unsigned int width, unsigned int height, QString key, const bool depthBuffer)
{
    m_mainMutex.lock();
    if (depthBuffer) {
        emit getGLDepthBuffer(width, height, key);
    } else {
        emit getGLImage(width, height, key);
    }
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
    str = QString( QByteArray(qBuffer.data(), int(qBuffer.size())).toBase64() );

    m_glImageIsReady = false;
    m_waitMutex.unlock();
    m_mainMutex.unlock();
}


void OpenGLServerGrabber::getImageAsText(QTextStream &os, unsigned int width, unsigned int height, QString key)
{
    QString rgbStr;
    getImageAsTextCommon(rgbStr, width, height, key, false);
    QString depthStr;
    getImageAsTextCommon(depthStr, width, height, key, true);

    // Is there something wrong with m_job->getExposedModel()?! Seems not to be returning meaningful data. (Or is it 'getElementValueAsString'?)
    std::string tmp = m_job->getExposedModel()->getElementValueAsString( key.toStdString() );
    //std::cout << "tmp=" << tmp << std::endl;

    tinia::model::Viewer viewer;
    m_job->getExposedModel()->getElementValue( key.toStdString(), viewer );
    //std::cout << "viewer.timestamp=" << viewer.timestamp << std::endl; // And this seems to be 0 all the time. Is that correct?
    std::stringstream ss;
    ss << viewer.modelviewMatrix[0] << " "
                                    << viewer.modelviewMatrix[1] << " "
                                    << viewer.modelviewMatrix[2] << " "
                                    << viewer.modelviewMatrix[3] << " "
                                    << viewer.modelviewMatrix[4] << " "
                                    << viewer.modelviewMatrix[5] << " "
                                    << viewer.modelviewMatrix[6] << " "
                                    << viewer.modelviewMatrix[7] << " "
                                    << viewer.modelviewMatrix[8] << " "
                                    << viewer.modelviewMatrix[9] << " "
                                    << viewer.modelviewMatrix[10] << " "
                                    << viewer.modelviewMatrix[11] << " "
                                    << viewer.modelviewMatrix[12] << " "
                                    << viewer.modelviewMatrix[13] << " "
                                    << viewer.modelviewMatrix[14] << " "
                                    << viewer.modelviewMatrix[15];
    //std::cout << "viewer.modelView: " << ss.str() << std::endl;
    QString viewStr( ss.str().c_str() );

    std::stringstream ss2;
    ss2 << viewer.projectionMatrix[0] << " "
                                    << viewer.projectionMatrix[1] << " "
                                    << viewer.projectionMatrix[2] << " "
                                    << viewer.projectionMatrix[3] << " "
                                    << viewer.projectionMatrix[4] << " "
                                    << viewer.projectionMatrix[5] << " "
                                    << viewer.projectionMatrix[6] << " "
                                    << viewer.projectionMatrix[7] << " "
                                    << viewer.projectionMatrix[8] << " "
                                    << viewer.projectionMatrix[9] << " "
                                    << viewer.projectionMatrix[10] << " "
                                    << viewer.projectionMatrix[11] << " "
                                    << viewer.projectionMatrix[12] << " "
                                    << viewer.projectionMatrix[13] << " "
                                    << viewer.projectionMatrix[14] << " "
                                    << viewer.projectionMatrix[15];
    //std::cout << "viewer.projection: " << ss2.str() << std::endl;
    QString projStr( ss2.str().c_str() );

    QString jsonWrappedImgPlusDepthString = "{ \"rgb\": \"" + rgbStr + "\", \"depth\": \"" + depthStr + "\", \"view\": \"" + viewStr+ "\", \"proj\": \"" + projStr + "\" }";
    os << httpHeader(getMimeType("file.txt")) << "\r\n" << jsonWrappedImgPlusDepthString;
}


void OpenGLServerGrabber::getImageCommon(unsigned int width, unsigned int height, QString key, const bool depthBufferRequested)
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

    glPixelStorei( GL_PACK_ALIGNMENT, 1 );

    if (depthBufferRequested) {
        glReadPixels( 0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, m_buffer );
#if 1
        // @@@ We record touched fragments in the blue channel, and use r+g as a 16 bit fixed point depth
        std::vector<unsigned char> tmp(width*height*3);
        glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, &(tmp[0]) );
        for(unsigned i = 0; i < width*height; i++) {
            float value = ((float*)m_buffer)[i];
            m_buffer[3*i    ] = value*255;         value -= m_buffer[3*i   ] * 255.0;
            m_buffer[3*i + 1] = value*255;
            m_buffer[3*i + 2] = 255 * ( (tmp[3*i]!=0) || (tmp[3*i+1]!=0) || (tmp[3*i+2]!=0) );
        }
#else
        // Depth encoded as 24 bit fixed point values.
        for(unsigned i = 0; i < width*height; i++) {
            float value = ((float*)m_buffer)[i];
            // 24
            m_buffer[3*i    ] = value*255;         value -= m_buffer[3*i   ] * 255.0;
            m_buffer[3*i + 1] = value*255;         value -= m_buffer[3*i +1] * 255.0;
            m_buffer[3*i + 2] = value*255;
        }
#endif
    } else {
        glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, m_buffer );
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    emit glImageReady();
}


// This is the slot for the signal OpenGLServerGrabber::getGLImage
void OpenGLServerGrabber::getImage(unsigned int width, unsigned int height, QString key)
{
    getImageCommon(width, height, key, false);
}


// This is the slot for the signal OpenGLServerGrabber::getGLDepthBuffer
void OpenGLServerGrabber::getDepthBuffer(unsigned int width, unsigned int height, QString key)
{
    getImageCommon(width, height, key, true);
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
