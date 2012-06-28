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

#ifndef QTOBSERVER_QTOBSERVER_HPP
#define QTOBSERVER_QTOBSERVER_HPP
#include <memory>
#include "tinia/jobobserver/Observer.hpp"

class QMainWindow;
class QGLWidget;
namespace tinia {
namespace model {
    class ExposedModel;
} // of namespace model

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
    std::shared_ptr<model::ExposedModel>   m_model;
    bool                                    m_perf_mode;
    bool                                    m_renderlist_mode;
};

} // namespace qtobserver
} // namespace tinia
#endif // QTOBSERVER_QTOBSERVER_HPP
