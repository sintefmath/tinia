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
