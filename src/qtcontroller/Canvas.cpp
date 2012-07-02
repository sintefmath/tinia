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

#include "tinia/qtcontroller/moc/Canvas.hpp"
#include <iostream>
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <tinia/jobcontroller/OpenGLJob.hpp>

namespace tinia {
namespace qtcontroller {
namespace impl {

Canvas::Canvas( jobcontroller::OpenGLJob*                 openglJob,
                std::string                             key,
                std::string                             boundingBoxKey,
                const std::string&                      resetViewKey,
                std::shared_ptr<model::ExposedModel>   model,
                QWidget*                                parent,
                QGLWidget*                              share_widget,
                bool                                    perf_mode)
    : QGLWidget( QGL::DepthBuffer| QGL::DoubleBuffer | QGL::AlphaChannel,
                 parent,
                 share_widget ),
      m_key(key),
      m_boundingBoxKey(boundingBoxKey),
      m_resetViewKey(resetViewKey),
      m_model(model),
      m_job(openglJob),
      m_dsrv(NULL),
      m_last_fps_calc( QTime::currentTime() ),
      m_frames( 0 ),
      m_redraw_timer( NULL ),
      m_render_mode(0),
      m_renderlist_db( NULL ),
      m_renderlist_renderer( NULL )
{
    model::Viewer viewer;
    m_model->getElementValue(m_key, viewer);
    setPreferredSize();
    initializeDSRV();

    if( share_widget != NULL) {
        if( !isSharing() ) {
            throw std::runtime_error( "Failed to enable sharing" );
        }
    }

    connect(this, SIGNAL(updateFromExposedModel()), this, SLOT(updateGL()));
    connect(this, SIGNAL(updateDSRV()), this, SLOT(updateDSRVNow()));
    connect(this, SIGNAL(resetViewFromExposedModel()), this, SLOT(resetView()));
    m_model->addStateListener(this);
    makeCurrent();

    if( perf_mode ) {
        // redraw whenever the event queue is empty
        m_redraw_timer = new QTimer;
        connect( m_redraw_timer, SIGNAL(timeout()), this, SLOT(updateGL()) );
        m_redraw_timer->setInterval(0);
        m_redraw_timer->setSingleShot(false);
        m_redraw_timer->start();
    }

    setFocusPolicy(Qt::ClickFocus);
}

Canvas::~Canvas()
{
    m_model->removeStateListener(this);
    if( m_redraw_timer != NULL ) {
        delete m_redraw_timer;
    }
}


void Canvas::paintGL()
{
    m_frames++;
    QTime current_time = QTime::currentTime();
    int msecs = m_last_fps_calc.msecsTo( current_time );
    if( msecs > 1000 ) {
        double fps = ((double)m_frames / (double)msecs)*1000.0;
        QString fps_str;
        fps_str.sprintf( "%.1f FPS", fps );
        emit updateFPS( fps_str );
        m_last_fps_calc = current_time;
        m_frames = 0;
    }
    if( m_render_mode == 0 ) {
        m_job->renderFrame("", m_key, 0, width(), height());
    }
    else if( m_render_mode == 1 ) {
        //if( m_renderlist_db == NULL ) {
        m_renderlist_db = m_job->getRenderList( "", m_key );
        //}
        if( m_renderlist_db == NULL ) {
            m_render_mode = 0;
        }
        else {
            if( m_renderlist_renderer == NULL ) {
                m_renderlist_renderer = new renderlist::gl::Renderer( *m_renderlist_db );
            }

            model::Viewer viewer;
            m_model->getElementValue( m_key, viewer);

            const float* p = viewer.projectionMatrix.data();
            glm::mat4 P( p[0], p[1], p[2], p[3],
                         p[4], p[5], p[6], p[7],
                         p[8], p[9], p[10], p[11],
                         p[12], p[13], p[14], p[15] );
            glm::mat4 Pi = glm::inverse( P );
            const float* m = viewer.modelviewMatrix.data();
            glm::mat4 M( m[0], m[1],  m[2], m[3],
                         m[4], m[5],  m[6], m[7],
                         m[8], m[9], m[10], m[11],
                         m[12], m[13], m[14], m[15] );
            glm::mat4 Mi = glm::inverse( M );

            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            m_renderlist_renderer->pull();
            m_renderlist_renderer->render( 0,
                                           viewer.projectionMatrix.data(),
                                           glm::value_ptr( Pi ),
                                           viewer.modelviewMatrix.data(),
                                           glm::value_ptr( Mi ),
                                           width(),
                                           height() );
        }
    }
}

void Canvas::resizeGL(int w, int h)
{

    model::Viewer viewer;
    m_model->getElementValue(m_key, viewer);
    viewer.width = w;
    viewer.height = h;
    m_model->updateElement(m_key, viewer);
    updateDSRV();

    updateGL();

}

void Canvas::setRenderMode( int index )
{
    m_render_mode = index;
    updateGL();
    //if( !perf_mode ) {
    /*// If not
        m_redraw_timer = new QTimer;
        connect( m_redraw_timer, SIGNAL(timeout()), this, SLOT(updateGL()) );
        m_redraw_timer->setInterval(0);
        m_redraw_timer->setSingleShot(false);
        m_redraw_timer->start();
        */
    //}
}

void Canvas::resetView()
{
    // Set new bounding boxes
    updateDSRVNow();
    m_dsrv->viewAll();
}

void Canvas::initializeGL()
{

}

void Canvas::setPreferredSize()
{
    model::Viewer viewer;
    m_model->getElementValue(m_key, viewer);
    resize(std::max(viewer.width, 640), std::max(viewer.height, 360));
}

void Canvas::resizeEvent(QResizeEvent *event)
{
    QGLWidget::resizeEvent(event);

    if(m_job->passThrough())
    {
        m_job->resizeEvent(event);
    }


    setPreferredSize();
}


QSizePolicy Canvas::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

QSize Canvas::minimumSize() const
{
    model::Viewer viewer;
    m_model->getElementValue(m_key, viewer);
    return QSize(viewer.width, viewer.height);
}

void Canvas::mousePressEvent(QMouseEvent *event)
{

    if(m_job->passThrough())
    {
        m_job->mousePressEvent(event);
        emit updateFromExposedModel();
    }

    switch(event->button())
    {
    case Qt::LeftButton:
        m_dsrv->startMotion(siut2::dsrv::DSRViewer::ROTATE, event->x(), event->y());
        break;
    case Qt::MiddleButton:
        m_dsrv->startMotion(siut2::dsrv::DSRViewer::PAN, event->x(), event->y());
        break;
    case Qt::RightButton:
        m_dsrv->startMotion(siut2::dsrv::DSRViewer::ZOOM, event->x(), event->y());
        break;
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{

    if(m_job->passThrough())
    {
        m_job->mouseMoveEvent(event);
    }

    m_dsrv->motion(event->x(), event->y());
    updateMatrices();
    updateGL();
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{

    if(m_job->passThrough())
    {
        m_job->mouseReleaseEvent(event);
    }

    m_dsrv->endMotion(event->x(), event->y());
    updateMatrices();
    updateGL();
}

void Canvas::keyPressEvent(QKeyEvent *event)
{

    if(m_job->passThrough())
    {
        m_job->keyPressEvent(event);
        updateGL();
    }

}

void Canvas::keyReleaseEvent(QKeyEvent *event)
{

    if(m_job->passThrough())
    {
        m_job->keyReleaseEvent(event);
        updateGL();
    }

}

void Canvas::initializeDSRV()
{
    // Defaulting to unit cube
    glm::vec3 max(1,1,1);
    glm::vec3 min(0,0,0);
    std::cerr<<"CHECKING FOR BOUNDINGBOXKEY="<<m_boundingBoxKey<<std::endl;
    if(m_model->hasElement(m_boundingBoxKey))
    {

        // (Not really) quick (and absolutely) dirty string to double-conversion
        std::string bbString = m_model->getElementValueAsString(m_boundingBoxKey);
        std::cerr<<"BOUNDINGBOX FROM POLICY: "<< bbString<<std::endl;
        QString helperString(bbString.c_str());
        QStringList splitString = helperString.split(' ');
        int index = 0;
        for(auto it = splitString.begin(); it!= splitString.end(); it++)
        {
            if(index < 3)
            {
                min[index++] = it->toDouble();
            }
            else
            {
                max[(index++)%3] = it->toDouble();
            }
        }
    }

    if(m_dsrv == NULL)
    {
        m_dsrv = new siut2::dsrv::DSRViewer(min, max);

        m_dsrv->setWindowSize(width(), height());
    }
    else
    {
        m_dsrv->updateViewVolume(min, max);
        m_dsrv->setWindowSize(width(), height());
    }
    updateMatrices();
}

void Canvas::updateMatrices()
{
    model::Viewer viewer;
    glm::mat4 modelView = m_dsrv->getModelviewMatrix();
    glm::mat4 projection = m_dsrv->getProjectionMatrix();
    m_model->getElementValue(m_key, viewer);

    // Doing this the hard way
    for(int i = 0; i < 4; i++)
    {
        for (int j= 0; j < 4; j++)
        {
            viewer.modelviewMatrix[i*4+j] = modelView[i][j];
            viewer.projectionMatrix[i*4+j] = projection[i][j];

        }
    }

    m_model->updateElement(m_key, viewer);
}

void Canvas::stateElementModified(model::StateElement *stateElement)
{
    if(stateElement->getKey() == m_boundingBoxKey)
    {
        emit updateDSRV();
    }
    else if(stateElement->getKey() == m_resetViewKey) {
        emit resetViewFromExposedModel();
    }
    emit updateFromExposedModel();
}

void Canvas::updateDSRVNow()
{
    initializeDSRV();
}

}
}
}
