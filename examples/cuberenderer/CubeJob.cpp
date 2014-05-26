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
#include "CubeJob.hpp"
#include "tinia/model/GUILayout.hpp"
#include <iostream>
#include "tinia/model/File.hpp"

namespace tinia {
namespace example {
CubeJob::CubeJob()
{
}


bool CubeJob::init()
{
    tinia::model::Viewer viewer;
    m_model->addElement("viewer", viewer);
    m_model->addElement<std::string>( "boundingbox", "-2.0 -2.0 -2.0 2.0 2.0 2.0" );

    // Adding variables to the model
    {
        m_model->addElement<bool>( "debugSplatCol", false );
        m_model->addAnnotation("debugSplatCol", "Index coloring (r, g, b, y, c, m)");
        m_model->addElement<bool>( "decayMode", false );
        m_model->addAnnotation("decayMode", "Splats decaying from center");
        m_model->addElement<bool>( "roundSplats", false );
        m_model->addAnnotation("roundSplats", "Circular splats");
        m_model->addElement<bool>( "screenSpaceSized", true );
        m_model->addAnnotation("screenSpaceSized", "Screen-space-sized splats");
        m_model->addConstrainedElement<int>("overlap", 100, 1, 300);
        m_model->addAnnotation("overlap", "Overlap factor)");
        m_model->addElement<bool>( "alwaysShowMostRecent", false );
        m_model->addAnnotation("alwaysShowMostRecent", "Always show most recent proxy model");
        m_model->addConstrainedElement<int>("mostRecentOffset", 0, 0, 100);
        m_model->addAnnotation("mostRecentOffset", "Offset (%%))");
        m_model->addElement<bool>( "transpBackground", false );
        m_model->addAnnotation("transpBackground", "Background in rgbTexture made transparent");
        m_model->addConstrainedElement<int>("splats", 16, 2, 512);
        m_model->addAnnotation("splats", "Number of splats)");
        m_model->addElement<bool>( "resetAllModels", false );
        m_model->addAnnotation("resetAllModels", "Remove all models, and update just once");
        m_model->addElement<bool>( "splatSizeLimiting", false );
        m_model->addAnnotation("splatSizeLimiting", "Splat size clamping (x3)");
        m_model->addElement<bool>( "fragDepthTest", false );
        m_model->addAnnotation("fragDepthTest", "fragDepthTest");
        m_model->addElement<bool>( "ignoreIntraSplatTexCoo", false );
        m_model->addAnnotation("ignoreIntraSplatTexCoo", "Ignore intra-splat texcoo");
        m_model->addElement<bool>( "splatOutline", true );
        m_model->addAnnotation("splatOutline", "Square splat outline");
//        m_model->addElement<bool>( "adjustTCwithFactorFromVS", false );
//        m_model->addAnnotation("adjustTCwithFactorFromVS", "Adjust tc in FS with factor from VS");
//        m_model->addElement<bool>( "differentiationTestFlag", false );
//        m_model->addAnnotation("differentiationTestFlag", "differentiationTestFlag");
    }

    // Setting up the mainGrid containing the GUI elements
    tinia::model::gui::Grid *mainGrid = new tinia::model::gui::Grid(100, 4);
    {
        int row = 0;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("debugSplatCol"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("decayMode"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("roundSplats"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("screenSpaceSized"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::HorizontalSlider("overlap"));
        mainGrid->setChild(row, 1, new tinia::model::gui::Label("overlap", false));
        mainGrid->setChild(row, 2, new tinia::model::gui::Label("overlap", true));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("alwaysShowMostRecent"));
        mainGrid->setChild(row, 1, new tinia::model::gui::HorizontalSlider("mostRecentOffset"));
        mainGrid->setChild(row, 2, new tinia::model::gui::Label("mostRecentOffset", false));
        mainGrid->setChild(row, 3, new tinia::model::gui::Label("mostRecentOffset", true));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("transpBackground"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::HorizontalSlider("splats"));
        mainGrid->setChild(row, 1, new tinia::model::gui::Label("splats", false));
        mainGrid->setChild(row, 2, new tinia::model::gui::Label("splats", true));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::Button("resetAllModels"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("splatSizeLimiting"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("fragDepthTest"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("ignoreIntraSplatTexCoo"));
        row++;
        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("splatOutline"));
        row++;
//        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("adjustTCwithFactorFromVS"));
//        row++;
//        mainGrid->setChild(row, 0, new tinia::model::gui::CheckBox("differentiationTestFlag"));
//        row++;
        // More elements...
    }

    // Setting up root consisting of canvas + mainGrid
    {
        tinia::model::gui::HorizontalLayout *rootLayout = new tinia::model::gui::HorizontalLayout();
        {
            tinia::model::gui::Canvas *canvas = new tinia::model::gui::Canvas("viewer", "renderlist", "boundingbox" );
            rootLayout->addChild(canvas);
        }
        rootLayout->addChild(mainGrid);
        m_model->setGUILayout(rootLayout, tinia::model::gui::DESKTOP);
    }

    return true;
}


CubeJob::~CubeJob()
{
}

void CubeJob::stateElementModified(tinia::model::StateElement *stateElement)
{
}

bool CubeJob::renderFrame(const std::string &session, const std::string &key, unsigned int fbo, const size_t width, const size_t height)
{
    //usleep(200000);
    //usleep(10000);

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
//    glColor3f(   1.0,  1.0,  0.0 );

    glColor3f(   1.0,  0.5,  0.0 );
    glVertex3f( -0.5, -0.5,  0.5 );

    glColor3f(   0.0,  1.0,  0.5 );
    glVertex3f( -0.5,  0.5,  0.5 );

    glColor3f(   1.0,  0.0,  0.5 );
    glVertex3f( -0.5,  0.5, -0.5 );

    glColor3f(   0.5,  0.0,  0.5 );
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
CubeJob::getRenderList( const std::string& session, const std::string& key )
{
    return &m_renderlist_db;
}
}
}
