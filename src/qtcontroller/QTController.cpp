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

#include "tinia/jobcontroller/Job.hpp"
#include "tinia/jobcontroller/OpenGLJob.hpp"
#include "tinia/qtcontroller/QTController.hpp"
#include "tinia/qtcontroller/GUIBuilder.hpp"
#include "tinia/qtcontroller/moc/HTTPServer.hpp"
#include <tinia/qtcontroller/scripting/utils.hpp>
#include <QApplication>
#include <QLayout>
#include <QLabel>
#include <QGLWidget>
#include <QMainWindow>
#include <exception>

#include <QDomDocument>
#include <QString>

// This needs to be outside namespaces because QT says so...
static void tiniaInitResources() {
    Q_INIT_RESOURCE(resources);
}

namespace tinia {
namespace qtcontroller {

QTController::QTController()
    : m_root_context(NULL),
      m_job(NULL),      
      m_perf_mode( false ),
      m_renderlist_mode( false )
{

}

QTController::~QTController()
{
    ;
}




void QTController::setJob(jobcontroller::Job *job)
{
    if( m_job != NULL ) {
        throw std::runtime_error( "Job already set" );
    }
    else {
       m_job = job;
    }
}

void QTController::notify()
{
}

void QTController::fail()
{
}

void QTController::finish()
{
}

void QTController::addScript(const std::string &script)
{
    m_scriptsToParse.push_back(script);
}

int QTController::run(int argc, char **argv)
{
    if( m_job == NULL ) {
        throw std::runtime_error( "No job set" );
    }
    m_model = m_job->getExposedModel();



    m_app.reset(  new QApplication( argc, argv ) );

    m_main_window.reset( new QMainWindow() );

    HTTPServer server(m_app.get());
    // Now we may init the script.
    tiniaInitResources();
    initScript();

    if( dynamic_cast<jobcontroller::OpenGLJob*>( m_job ) ) {
        for( int i=1; i<argc; i++) {
            if( strcmp(argv[i], "--perf" ) == 0 ) {
                m_perf_mode = true;
            }
            else if( strcmp( argv[i], "--renderlist" ) == 0 ) {
                m_renderlist_mode = true;
            }
        }

        // Create an off-screen context that is subsequently shared with all
        // GL widgets that will be subsequently created. This avoids requiring
        // that the view is defined before doing GL init as well as allowing
        // GL resources to be shared between widgets.
        m_root_context = new QGLWidget(QGL::DepthBuffer| QGL::DoubleBuffer | QGL::AlphaChannel);
        m_root_context->makeCurrent();
    }
    if( !m_job->init() ) {
       throw new std::runtime_error("Job did not start up properly");
    }

    m_builder.reset( new GUIBuilder( m_model, m_job, this, m_perf_mode, m_root_context ) );

    m_main_window->setCentralWidget( m_builder->buildGUI( m_model->getGUILayout(model::gui::DESKTOP),
                                                          NULL ) );

	if( dynamic_cast<jobcontroller::OpenGLJob*>( m_job ) ) {
	if( !(dynamic_cast<jobcontroller::OpenGLJob*>(m_job))->initGL()) {
		throw new std::runtime_error("Job did not initialize GL correctly");
	}
	}
    m_main_window->show();

    return m_app->exec();
}

void QTController::initScript()
{
    scripting::addDefaultScripts(scripting::scriptEngineInstance());
    for(size_t i = 0; i < m_scriptsToParse.size(); ++i) {

        auto error = scripting::scriptEngineInstance().evaluate(QString(m_scriptsToParse[i].c_str()));
        if(error.isError()) {
            throw std::runtime_error("Error parsing script: "
                                     + error.toString().toStdString()
                                     + "\n\nSCRIPT:\n"
                                     + m_scriptsToParse[i]);
        }
    }
}

}// namespace qtcontroller
} // of namespace tinia
