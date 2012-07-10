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


class QMainWindow;
class QGLWidget;
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

    QMainWindow*                            m_main_window;
    QGLWidget*                              m_root_context;
    jobcontroller::Job*                       m_job;
    GUIBuilder*                             m_builder;
    std::shared_ptr<model::ExposedModel>   m_model;
    bool                                    m_perf_mode;
    bool                                    m_renderlist_mode;
};

} // namespace qtcontroller
} // namespace tinia

