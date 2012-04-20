#ifndef QTOBSERVER_QTOBSERVER_HPP
#define QTOBSERVER_QTOBSERVER_HPP
#include <memory>
#include "tinia/jobobserver/Observer.hpp"

class QMainWindow;
class QGLWidget;
namespace tinia {
namespace policylib {
    class PolicyLib;
} // of namespace policylib

namespace jobobserver {
    class Job;
} // of namespace jobobserver

namespace qtobserver {
    class GUIBuilder;

class QTObserver : public jobobserver::Observer
{
public:
    QTObserver();
    ~QTObserver();
    void setJob(jobobserver::Job* job);

    void init();

    void notify();
    void fail();
    void finish();

    bool perfMode() const { return m_perf_mode; }
    bool renderListMode() const { return m_renderlist_mode; }

    /**
      *
      * Recognized arguements:
      * * --perf Continuous redraw with FPS counter
      * * --renderlist Allow renderering from render lists.
      *
      */
    int run(int argc, char** argv);
private:
    void populateGUI();

    QMainWindow*                            m_main_window;
    QGLWidget*                              m_root_context;
    jobobserver::Job*                       m_job;
    GUIBuilder*                             m_builder;
    std::shared_ptr<policylib::PolicyLib>   m_policyLib;
    bool                                    m_perf_mode;
    bool                                    m_renderlist_mode;
};

} // namespace qtobserver
} // namespace tinia
#endif // QTOBSERVER_QTOBSERVER_HPP
