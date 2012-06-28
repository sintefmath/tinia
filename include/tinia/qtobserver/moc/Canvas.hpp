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

#ifndef QTOBSERVER_CANVAS_HPP
#define QTOBSERVER_CANVAS_HPP
#include <QtOpenGL>
#include <QGLWidget>
#include <QSizePolicy>
#include <QString>
#include <QTime>
#include <QTimer>
#include <tinia/renderlist/gl/Renderer.hpp>
#include <siut2/dsrv/DSRViewer.hpp>
#include "tinia/jobobserver/OpenGLJob.hpp"
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/StateListener.hpp"

namespace tinia {
namespace qtobserver {

class Canvas : public QGLWidget, public policy::StateListener
{
    Q_OBJECT
public:
    explicit Canvas(jobobserver::OpenGLJob* openglJob,
                    std::string key, std::string boundingBoxKey,
                    const std::string& resetViewKey,
                    std::shared_ptr<policy::Policy> policy,
                    QWidget* parent,
                    QGLWidget* share_widget,
                    bool perf_mode = false);
   ~Canvas();

   void stateElementModified(policy::StateElement *stateElement);
   void setPreferredSize();
   QSizePolicy sizePolicy() const;
   QSize minimumSize() const;
protected:
   void paintGL();
   void resizeGL(int w, int h);
   void initializeGL();
   void resizeEvent(QResizeEvent *event);
   void mousePressEvent(QMouseEvent *event);
   void mouseMoveEvent(QMouseEvent *event);
   void mouseReleaseEvent(QMouseEvent *event);
   void keyPressEvent(QKeyEvent *event);
   void keyReleaseEvent(QKeyEvent *);
signals:
   void resetViewFromPolicy();
   void updateFromPolicy();
   void updateDSRV();
   void updateFPS( QString fps );
public slots:
   void updateDSRVNow();
   void setRenderMode( int index );
   void resetView();
private:
   void updateMatrices();
   void initializeDSRV();
   std::string m_key;
   std::string m_boundingBoxKey;
   std::string m_resetViewKey;
   std::shared_ptr<policy::Policy> m_policy;
   jobobserver::OpenGLJob* m_job;
   siut2::dsrv::DSRViewer* m_dsrv;
   QTime                   m_last_fps_calc;
   unsigned int            m_frames;
   QTimer*                 m_redraw_timer;
   int                              m_render_mode;
   const renderlist::DataBase*   m_renderlist_db;
   renderlist::gl::Renderer*     m_renderlist_renderer;
};

} // namespace qtobserver
} // namespace tinia
#endif // QTOBSERVER_CANVAS_HPP
