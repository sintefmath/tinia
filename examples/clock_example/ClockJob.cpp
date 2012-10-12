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

#include <GL/glew.h>
#include <tinia/renderlist/Buffer.hpp>
#include <tinia/renderlist/Draw.hpp>
#include <tinia/renderlist/SetViewCoordSys.hpp>
#include <tinia/renderlist/Shader.hpp>
#include <tinia/renderlist/SetShader.hpp>
#include <tinia/renderlist/SetInputs.hpp>
#include <tinia/renderlist/SetUniforms.hpp>
#include <tinia/renderlist/SetLight.hpp>
#include <tinia/renderlist/SetLocalCoordSys.hpp>
#include <tinia/renderlist/SetFramebuffer.hpp>
#include <tinia/renderlist/SetFramebufferState.hpp>
#include <tinia/renderlist/SetPixelState.hpp>
#include <tinia/renderlist/SetRasterState.hpp>
#include "utils.hpp"
#include "ClockJob.hpp"
#include "tinia/model/GUILayout.hpp"
#include <iostream>
#include "tinia/model/File.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace tinia {
namespace example {
namespace {
class Timer {
public:
    Timer(std::shared_ptr<tinia::model::ExposedModel>& exposedModel) : m_model(exposedModel) {
        m_model->addElement<std::string>("time", "0");
        m_model->addElement<int>("elapsed", 0);
    }

    void operator()() {
        while(true) {
            boost::this_thread::sleep(boost::posix_time::millisec(500));
            boost::posix_time::ptime currentTime = boost::posix_time::ptime(boost::posix_time::second_clock::local_time());

            auto currentSeconds = currentTime.time_of_day().total_seconds();

            m_model->updateElement<int>("elapsed", currentSeconds);


            m_model->updateElement<std::string>("time",
                                                boost::lexical_cast<std::string>(
                                                boost::posix_time::second_clock::local_time()));


        }
    }

private:
    std::shared_ptr<tinia::model::ExposedModel> m_model;
};
}
ClockJob::ClockJob() : m_timeThread(Timer(m_model))
{


}

bool ClockJob::init()
{

    tinia::model::Viewer viewer;
    m_model->addElement("viewer", viewer);
    m_model->addElement<std::string>( "boundingbox", "-2.0 -2.0 -2.0 2.0 2.0 2.0" );

    auto layout = new tinia::model::gui::VerticalLayout();
    layout->addChild(new tinia::model::gui::Label("time", true));

    layout->addChild(new tinia::model::gui::Canvas("viewer", "renderlist", "boundingbox"));
    m_model->setGUILayout(layout,
                          tinia::model::gui::DESKTOP);

    return true;
}

ClockJob::~ClockJob()
{

}

void ClockJob::stateElementModified(tinia::model::StateElement *stateElement)
{
}

bool ClockJob::renderFrame(const std::string &session, const std::string &key, unsigned int fbo, const size_t width, const size_t height)
{
    glEnable(GL_DEPTH_TEST);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, width, height);
    tinia::model::Viewer viewer;
    m_model->getElementValue( key, viewer);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf( viewer.projectionMatrix.data() );


    auto elapsed = m_model->getElementValue<int>("elapsed");
    auto modelview =glm::rotate(glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -3.f)),
                                               float(elapsed)*10.f, glm::vec3(0.f, 1.f,0.f));

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf( glm::value_ptr(modelview));

    glBegin(GL_POLYGON);
    glColor3f(   1.0,  0.0, 0.0 );
    glVertex3f(  0.5, -0.5, -0.5 );
    glVertex3f(  0.5,  0.5, -0.5 );
    glVertex3f( -0.5,  0.5, -0.5 );
    glVertex3f( -0.5, -0.5, -0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(   0.0,  1.0, 0.0 );
    glVertex3f(  0.5, -0.5, 0.5 );
    glVertex3f(  0.5,  0.5, 0.5 );
    glVertex3f( -0.5,  0.5, 0.5 );
    glVertex3f( -0.5, -0.5, 0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(  .0,  0.0,  1.0 );
    glVertex3f( 0.5, -0.5, -0.5 );
    glVertex3f( 0.5,  0.5, -0.5 );
    glVertex3f( 0.5,  0.5,  0.5 );
    glVertex3f( 0.5, -0.5,  0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(   1.0,  1.0,  0.0 );
    glVertex3f( -0.5, -0.5,  0.5 );
    glVertex3f( -0.5,  0.5,  0.5 );
    glVertex3f( -0.5,  0.5, -0.5 );
    glVertex3f( -0.5, -0.5, -0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(   1.0,  0.0,  1.0 );
    glVertex3f(  0.5,  0.5,  0.5 );
    glVertex3f(  0.5,  0.5, -0.5 );
    glVertex3f( -0.5,  0.5, -0.5 );
    glVertex3f( -0.5,  0.5,  0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(   0.0,  1.0,  1.0 );
    glVertex3f(  0.5, -0.5, -0.5 );
    glVertex3f(  0.5, -0.5,  0.5 );
    glVertex3f( -0.5, -0.5,  0.5 );
    glVertex3f( -0.5, -0.5, -0.5 );
    glEnd();
    CHECK_GL;

    return true;
}

const tinia::renderlist::DataBase*
ClockJob::getRenderList( const std::string& session, const std::string& key )
{
    return NULL;
}
}
}
