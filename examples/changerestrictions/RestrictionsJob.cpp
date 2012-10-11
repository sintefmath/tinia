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
#include "RestrictionsJob.hpp"
#include "tinia/model/GUILayout.hpp"
#include <iostream>
#include "tinia/model/File.hpp"

namespace tinia {
namespace example {
RestrictionsJob::RestrictionsJob()
{
    m_possibleChoices.push_back("one");
    m_possibleChoices.push_back("two");
    m_possibleChoices.push_back("three");
    m_possibleChoices.push_back("four");

    m_model->addStateListener(this);
}

bool RestrictionsJob::init()
{

    tinia::model::Viewer viewer;
    m_model->addElement("viewer", viewer);
    m_model->addElement<std::string>( "boundingbox", "-2.0 -2.0 -2.0 2.0 2.0 2.0" );

    for(size_t i = 0u; i < m_possibleChoices.size(); ++i) {
        m_model->addElement<bool>(m_possibleChoices[i], true);
    }

    m_model->addElementWithRestriction<std::string>("restricted", "one", m_possibleChoices);

    auto layout = new tinia::model::gui::VerticalLayout();

    for(size_t i = 1u; i < m_possibleChoices.size(); ++i) { // Don't want them to remove everything
        layout->addChild(new tinia::model::gui::CheckBox(m_possibleChoices[i]));
    }

    layout->addChild(new tinia::model::gui::ComboBox("restricted"));
    layout->addChild(new tinia::model::gui::RadioButtons("restricted"));

    layout->addChild(new tinia::model::gui::Canvas("viewer", "renderlist", "boundingbox"));
    m_model->setGUILayout(layout,
                          tinia::model::gui::DESKTOP);
    return true;
}

RestrictionsJob::~RestrictionsJob()
{
    m_model->removeStateListener(this);
}

void RestrictionsJob::stateElementModified(tinia::model::StateElement *stateElement)
{
    bool found = false;
    for(size_t i = 0; i < m_possibleChoices.size(); ++i) {
        if(m_possibleChoices[i] == stateElement->getKey()) {
            found = true;
        }
    }

    if(found) {
        std::vector<std::string> newRestriction;
        for(size_t i = 0u; i < m_possibleChoices.size(); ++i) {
            if(m_model->getElementValue<bool>(m_possibleChoices[i])) {
                newRestriction.push_back(m_possibleChoices[i]);
            }
        }
        m_model->updateRestrictions("restricted", newRestriction[0], newRestriction);
    }
}

bool RestrictionsJob::renderFrame(const std::string &session, const std::string &key, unsigned int fbo, const size_t width, const size_t height)
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
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf( viewer.modelviewMatrix.data() );

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
RestrictionsJob::getRenderList( const std::string& session, const std::string& key )
{
    return NULL;
}
}
}
