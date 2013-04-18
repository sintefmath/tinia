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
#pragma once
#include <memory>
#include "tinia/jobcontroller/Controller.hpp"
#include <tinia/qtcontroller/scripting/ScriptEngine.hpp>
#ifdef TINIA_HAVE_LIBXML
#include <tinia/qtcontroller/moc/ServerController.hpp>
#endif
#include <QToolBar>
#include <boost/scoped_ptr.hpp>

class QMainWindow;
class QGLWidget;
class QApplication;
namespace tinia {
namespace model {
    class ExposedModel;
} // of namespace model

namespace jobcontroller {
    class Job;
} // of namespace jobcontroller

namespace qtcontroller {
    class GUIBuilder;

class QTController : public jobcontroller::Controller
{
public:
    QTController();
    ~QTController();
    void setJob(jobcontroller::Job* job);

    void init();

    void notify();
    void fail();
    void finish();

    /** Adds the script to the main scripting engine
     * @param script
     */
    void addScript(const std::string& script);

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
    void initScript();
    void populateGUI();

    // The order of these two is important, to ensure proper destructor order.
    // App will be deleted after m_main_window.
    boost::scoped_ptr<QApplication>          m_app;
    boost::scoped_ptr<QMainWindow>           m_main_window;

#ifdef TINIA_HAVE_LIBXML // This is only for those who have libxml available
    impl::ServerController* m_serverController;
    QToolBar* m_toolBar; // Lifetime managed by qt parent-child machinery.

    void setupServerController();
#endif

    QGLWidget*                             m_root_context; // Lifetime managed by qt parent-child machinery.
    jobcontroller::Job*                    m_job;
    boost::shared_ptr<GUIBuilder>            m_builder;
    boost::shared_ptr<model::ExposedModel>   m_model;

    // We need to hold the scripts in memory untill we start up QApplication
    std::vector<std::string>                m_scriptsToParse;
    bool                                    m_perf_mode;
    bool                                    m_renderlist_mode;    
};

} // namespace qtcontroller
} // namespace tinia

