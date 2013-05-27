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
#include <tinia/jobcontroller/OpenGLJob.hpp>
#include <tinia/qtcontroller/scripting/ScriptEngine.hpp>

namespace tinia {
namespace qtcontroller {
namespace impl {

Canvas::Canvas( jobcontroller::OpenGLJob*                 openglJob,
                std::string                             key,
                const tinia::model::gui::ScriptArgument& viewerType,
                const std::vector<tinia::model::gui::ScriptArgument>& scripts,
                boost::shared_ptr<model::ExposedModel>   model,
                QWidget*                                parent,
                QGLWidget*                              share_widget,
                bool                                    perf_mode)
    : QGLWidget( QGL::DepthBuffer| QGL::DoubleBuffer | QGL::AlphaChannel,
                 parent,
                 share_widget ),

      m_perf_mode( perf_mode ),
      m_key(key),
      m_model(model),
      m_job(openglJob),
      m_last_fps_calc( QTime::currentTime() ),
      m_frames( 0 ),
      m_redraw_timer( NULL ),
      m_render_mode(0),
      m_renderlist_db( NULL ),
      m_renderlist_renderer( NULL )
{
    m_eventHandlers.push_back(boost::shared_ptr<scripting::EventHandler>(new scripting::EventHandler(viewerType.className(),
                                                                                                   viewerType.parameters(),
                                                                                                   model,
                                                                                                   scripting::scriptEngineInstance())));
    for(size_t i = 0; i < scripts.size(); ++i) {
        m_eventHandlers.push_back(boost::shared_ptr<scripting::EventHandler>(new scripting::EventHandler(scripts[i].className(),
                                                                                                       scripts[i].parameters(),
                                                                                                       model,
                                                                                                       scripting::scriptEngineInstance())));
    }
    model::Viewer viewer;
    m_model->getElementValue(m_key, viewer);
    setPreferredSize();

    if( share_widget != NULL) {
        if( !isSharing() ) {
            throw std::runtime_error( "Failed to enable sharing" );
        }
    }

    m_redraw_timer = new QTimer;
    connect( m_redraw_timer, SIGNAL(timeout()), this, SLOT(updateGL()) );
    m_redraw_timer->setInterval( m_perf_mode ? 0 : 10000000 );
    m_redraw_timer->setSingleShot(false);
    m_redraw_timer->start();

    connect(this, SIGNAL(updateFromExposedModel()), this, SLOT(triggerRedraw()));

    m_model->addStateListener(this);
    makeCurrent();

    if( perf_mode ) {
        // redraw whenever the event queue is empty
    }

    setFocusPolicy(Qt::ClickFocus);

    // Enable mouse movement tracking even when not holding mousebutton.
    setMouseTracking(true);
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
    m_redraw_timer->setInterval( m_perf_mode ? 0 : 10000000 );
    //m_redraw_timer->setInterval( 1000000 );
}

void Canvas::triggerRedraw()
{
    m_redraw_timer->setInterval( 0 );
}

void Canvas::resizeGL(int w, int h)
{

    model::Viewer viewer;
    m_model->getElementValue(m_key, viewer);
    viewer.width = w;
    viewer.height = h;
    m_model->updateElement(m_key, viewer);
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
    for(size_t i = 0; i < m_eventHandlers.size(); ++i) {
        m_eventHandlers[i]->mousePressEvent(event);
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    for(size_t i = 0; i < m_eventHandlers.size(); ++i) {
        m_eventHandlers[i]->mouseMoveEvent(event);
    }
    updateGL();
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    for(size_t i = 0; i < m_eventHandlers.size(); ++i) {
        m_eventHandlers[i]->mouseReleaseEvent(event);
    }
    updateGL();
}

void Canvas::keyPressEvent(QKeyEvent *event)
{
    for(size_t i = 0; i < m_eventHandlers.size(); ++i) {
        m_eventHandlers[i]->keyPressEvent(event);
    }
    updateGL();
}

void Canvas::keyReleaseEvent(QKeyEvent *event)
{
    ;
}




void Canvas::stateElementModified(model::StateElement *stateElement)
{
    emit updateFromExposedModel();
}

}
}
}
