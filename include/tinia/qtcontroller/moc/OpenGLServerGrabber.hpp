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

class OpenGLServerGrabber : public QObject, public ImageSource
{
    Q_OBJECT
public:
    explicit OpenGLServerGrabber(tinia::jobcontroller::Job* job,
                                 QObject *parent = 0);

    ~OpenGLServerGrabber();

    void getImageAsText(QTextStream& os, unsigned int width, unsigned int height, QString key);
    
signals:
    void glImageReady();
    void getGLImage(unsigned int width, unsigned int height, QString key);

private slots:
    void getImage(unsigned int width, unsigned int height, QString key);
    void wakeListeners();

private:
    void setupOpenGL();
    void resize(unsigned int width, unsigned int height);
    bool m_glImageIsReady;

    // We only want to grab one image at the time
    QMutex m_mainMutex;

    // This is for waiting. We let the event-loop take the image, and wait
    // for it to finish.
    QMutex m_waitMutex;
    QWaitCondition m_waitCondition;

    tinia::jobcontroller::Job* m_job;
    unsigned char* m_buffer;
    size_t          m_buffer_size;
    QString         m_renderlist_update_xml;
    bool m_openglIsReady;
    unsigned int m_fbo;
    unsigned int m_renderbufferRGBA;
    unsigned int m_renderbufferDepth;

    unsigned int m_width;
    unsigned int m_height;
};

} // namespace impl
} // namespace qtcontroller
} // namespace tinia

