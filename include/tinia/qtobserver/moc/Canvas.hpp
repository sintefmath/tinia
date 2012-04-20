#ifndef QTOBSERVER_CANVAS_HPP
#define QTOBSERVER_CANVAS_HPP
#include <QtOpenGL>
#include <QGLWidget>
#include <QSizePolicy>
#include <QString>
#include <QTime>
#include <QTimer>
#include <tinia/librenderlist/gl/Renderer.hpp>
#include <siut2/dsrv/DSRViewer.hpp>
#include "tinia/jobobserver/OpenGLJob.hpp"
#include "tinia/policylib/PolicyLib.hpp"
#include "tinia/policylib/StateListener.hpp"

namespace tinia {
namespace qtobserver {

class Canvas : public QGLWidget, public policylib::StateListener
{
    Q_OBJECT
public:
    explicit Canvas(jobobserver::OpenGLJob* openglJob,
                    std::string key, std::string boundingBoxKey,
                    std::shared_ptr<policylib::PolicyLib> policyLib,
                    QWidget* parent,
                    QGLWidget* share_widget,
                    bool perf_mode = false);
   ~Canvas();

   void stateElementModified(policylib::StateElement *stateElement);
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
   void updateFromPolicylib();
   void updateDSRV();
   void updateFPS( QString fps );
public slots:
   void updateDSRVNow();
   void setRenderMode( int index );
private:
   void updateMatrices();
   void initializeDSRV();
   std::string m_key;
   std::string m_boundingBoxKey;
   std::shared_ptr<policylib::PolicyLib> m_policyLib;
   jobobserver::OpenGLJob* m_job;
   siut2::dsrv::DSRViewer* m_dsrv;
   QTime                   m_last_fps_calc;
   unsigned int            m_frames;
   QTimer*                 m_redraw_timer;
   int                              m_render_mode;
   const librenderlist::DataBase*   m_renderlist_db;
   librenderlist::gl::Renderer*     m_renderlist_renderer;
};

} // namespace qtobserver
} // namespace tinia
#endif // QTOBSERVER_CANVAS_HPP
