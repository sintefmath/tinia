#pragma once

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QTextStream>
#include "tinia/jobcontroller.hpp"
#include "tinia/qtcontroller/ImageSource.hpp"

namespace tinia {
namespace qtcontroller {
namespace impl {

class OpenGLServerGrabber : public QObject
{
    Q_OBJECT
public:
    explicit OpenGLServerGrabber( tinia::jobcontroller::Job* job,
                                  QObject *parent );

    ~OpenGLServerGrabber();

private:

    // This is for waiting. We let the event-loop take the image, and wait
    // for it to finish.
   QWaitCondition m_waitCondition;

 tinia::jobcontroller::Job* m_job;

public:
    /** Mutex that governs exclusive access to this object. */
    QMutex*
    exclusiveAccessMutex()
    { return &m_mainMutex; }

    /** Returns a pointer to the grabbed image.
     *
     * \note \ref exclusiveAccessMutex must be held before invocation.
     */
    const unsigned char*
    imageBuffer() const
    { return m_buffer; }

    /** Grabs an image of a view
     *
     * \note Must be invoked in the thread that holds the OpenGL context,
     *       usually the main/GUI-thread.
     * \note \ref exclusiveAccessMutex must be held before invocation.
     */
    void
    grabRGB( tinia::jobcontroller::OpenGLJob* job,
             unsigned int width,
             unsigned int height,
             const std::string &key);
    void
    grabDepth( tinia::jobcontroller::OpenGLJob* job,
               unsigned int width,
               unsigned int height,
               const std::string &key);

private:
    void setupOpenGL();
    void resize(unsigned int width, unsigned int height);

    QMutex          m_mainMutex;
    unsigned char*  m_buffer;
    size_t          m_buffer_size;
    bool            m_openglIsReady;
    unsigned int    m_fbo;
    unsigned int    m_renderbufferRGBA;
    unsigned int    m_renderbufferDepth;
    unsigned int    m_width;
    unsigned int    m_height;
};

} // namespace impl
} // namespace qtcontroller
} // namespace tinia

