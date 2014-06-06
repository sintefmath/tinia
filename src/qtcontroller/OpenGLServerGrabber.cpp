#include "tinia/qtcontroller/moc/OpenGLServerGrabber.hpp"
#include <GL/glew.h>
#include <QImage>
#include <QBuffer>
#include <tinia/qtcontroller/impl/http_utils.hpp>
#include "tinia/renderlist.hpp"

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
    
    connect( this, SIGNAL(signalGetRenderListUpdate(QString)),
             this, SLOT(getRenderListUpdate(QString)) );
}

OpenGLServerGrabber::~OpenGLServerGrabber()
{
    if(m_openglIsReady) {
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteRenderbuffers(1, &m_renderbufferRGBA);
        glDeleteRenderbuffers(1, &m_renderbufferDepth);
    }
}

void OpenGLServerGrabber::getRenderListUpdateResponse( QTextStream& response,
                                                       const QString& request )
{
    m_mainMutex.lock();

    emit signalGetRenderListUpdate( request );

    m_waitMutex.lock();
    while(!m_glImageIsReady) {
        m_waitCondition.wait(&m_waitMutex);
    }

    response << httpHeader("application/xml") << "\r\n";
    response << m_renderlist_update_xml << "\n";

    m_glImageIsReady = false;
    m_waitMutex.unlock();
    m_mainMutex.unlock();
}

void OpenGLServerGrabber::getRenderListUpdate( const QString& request )
{
    // runs as main thread
    tinia::jobcontroller::OpenGLJob* openGLJob = static_cast<tinia::jobcontroller::OpenGLJob*>(m_job);
    if(!openGLJob) {
        throw std::invalid_argument("This is not an OpenGL job!");
    }

    typedef boost::tuple<std::string, unsigned int> params_t;    
    params_t params = parseGet<params_t>( decodeGetParameters(request),
                                          "key timestamp" );
    const tinia::renderlist::DataBase* db = openGLJob->getRenderList( "session",
                                                                      params.get<0>() );
    if(db) {
        std::string list = renderlist::getUpdateXML( db,
                                                     renderlist::ENCODING_JSON,
                                                     params.get<1>() );
        m_renderlist_update_xml = QString(list.c_str());
    }
    else {
        m_renderlist_update_xml.clear();
    }

    // notify waiting server thread that we are finished.
    m_waitMutex.lock();
    m_glImageIsReady = true;
    m_waitCondition.wakeAll();
    m_waitMutex.unlock();    
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
