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
#include "APCJob.hpp"
#include "tinia/model/GUILayout.hpp"
#include <iostream>
#include "tinia/model/File.hpp"
#include "tinia/utils/ProxyDebugGUI.hpp"



namespace tinia {
namespace example {
APCJob::APCJob()
{
}


bool APCJob::init()
{
    tinia::model::Viewer viewer;
    m_model->addElement("viewer", viewer);
    m_model->addElement<std::string>( "boundingbox", "-2.0 -2.0 -2.0 2.0 2.0 2.0" );

    // The constructor of proxyGUI sets up "magical" elements in m_model.
    tinia::utils::ProxyDebugGUI proxyGUI( m_model,
                                          true, // autoProxy
                                          true, // autoProxy-debugging
                                          true, // jpgProxy
                                          true  // autoSelect proxy method
                                          );

    // And this method returns a corresponding grid containing GUI elements for manipulating these elements.
    tinia::model::gui::Grid *grid = proxyGUI.getGrid();

    // Setting up root consisting of canvas + grid
    tinia::model::gui::HorizontalLayout *rootLayout = new tinia::model::gui::HorizontalLayout();
    {
        tinia::model::gui::Canvas *canvas = new tinia::model::gui::Canvas("viewer", "renderlist", "boundingbox" );
        rootLayout->addChild(canvas);
    }
    rootLayout->addChild(grid);
    m_model->setGUILayout(rootLayout, tinia::model::gui::DESKTOP);

    return true;
}


APCJob::~APCJob()
{
}

void APCJob::stateElementModified(tinia::model::StateElement *stateElement)
{
}

bool APCJob::renderFrame(const std::string &session, const std::string &key, unsigned int fbo, const size_t width, const size_t height)
{
    static bool firsttime = true;
    if (firsttime) {

        // 160803: Think this is for producing the fine network of black orthogonal lines used on the cube faces.

        std::cout << "Setting up texture" << std::endl;
        glGenTextures(1, &m_tex);
        glBindTexture(GL_TEXTURE_2D, m_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        const int n=512;
        std::vector<unsigned char> texData(n*n*3, 255);
        for (int i=0; i<n; i+=(n/16)) {
            for (int j=0; j<n; j++) {
                const unsigned char val = 0; // 160803: Hmm... why doesn't 255 produce white lines instead of black ones?! Blend mode issue.
                texData[3*(i*n+j) + 0] = val;
                texData[3*(i*n+j) + 1] = val;
                texData[3*(i*n+j) + 2] = val;
                texData[3*(j*n+i) + 0] = val;
                texData[3*(j*n+i) + 1] = val;
                texData[3*(j*n+i) + 2] = val;
            }
        }

        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

        glTexImage2D(GL_TEXTURE_2D,
                     0, // mipmap level
                     GL_RGB, n, n, 0, GL_RGB, GL_UNSIGNED_BYTE, &texData[0]);

        //glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        firsttime = false;
    }

    // Simulated high latency
    // usleep(200000);

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

#if 0

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, m_tex);

    // Coloured box with orthogonal network of thin lines in texture

    glBegin(GL_POLYGON);
    glColor3f(   1.0,  0.0, 0.0 );
    glTexCoord2f(1, 1);
    glVertex3f(  0.5, -0.5, -0.5 );
    glTexCoord2f(1, 0);
    glVertex3f(  0.5,  0.5, -0.5 );
    glTexCoord2f(0, 0);
    glVertex3f( -0.5,  0.5, -0.5 );
    glTexCoord2f(0, 1);
    glVertex3f( -0.5, -0.5, -0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(  .0,  0.0,  1.0 );
    glTexCoord2f(1, 1);
    glVertex3f( 0.5, -0.5, -0.5 );
    glTexCoord2f(1, 0);
    glVertex3f( 0.5,  0.5, -0.5 );
    glTexCoord2f(0, 0);
    glVertex3f( 0.5,  0.5,  0.5 );
    glTexCoord2f(0, 1);
    glVertex3f( 0.5, -0.5,  0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(   1.0,  0.5,  0.0 );
    glTexCoord2f(1, 1);
    glVertex3f( -0.5, -0.5,  0.5 );
    glColor3f(   0.0,  1.0,  0.5 );
    glTexCoord2f(1, 0);
    glVertex3f( -0.5,  0.5,  0.5 );
    glColor3f(   1.0,  0.0,  0.5 );
    glTexCoord2f(0, 0);
    glVertex3f( -0.5,  0.5, -0.5 );
    glColor3f(   0.5,  0.0,  0.5 );
    glTexCoord2f(0, 1);
    glVertex3f( -0.5, -0.5, -0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(   0.0,  1.0, 0.0 );
    glTexCoord2f(1, 1);
    glVertex3f(  0.5, -0.5, 0.5 );
    glTexCoord2f(1, 0);
    glVertex3f(  0.5,  0.5, 0.5 );
    glTexCoord2f(0, 0);
    glVertex3f( -0.5,  0.5, 0.5 );
    glTexCoord2f(0, 1);
    glVertex3f( -0.5, -0.5, 0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(   1.0,  0.0,  1.0 );
    glTexCoord2f(1, 1);
    glVertex3f(  0.5,  0.5,  0.5 );
    glTexCoord2f(1, 0);
    glVertex3f(  0.5,  0.5, -0.5 );
    glTexCoord2f(0, 0);
    glVertex3f( -0.5,  0.5, -0.5 );
    glTexCoord2f(0, 1);
    glVertex3f( -0.5,  0.5,  0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(   0.0,  1.0,  1.0 );
    glTexCoord2f(1, 1);
    glVertex3f(  0.5, -0.5, -0.5 );
    glTexCoord2f(1, 0);
    glVertex3f(  0.5, -0.5,  0.5 );
    glTexCoord2f(0, 0);
    glVertex3f( -0.5, -0.5,  0.5 );
    glTexCoord2f(0, 1);
    glVertex3f( -0.5, -0.5, -0.5 );
    glEnd();

    glDisable(GL_TEXTURE_2D);

#else

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glBegin(GL_POLYGON);
    glColor3f(   1.0,  0.0, 0.0 );
    glTexCoord2f(1, 1);
    glVertex3f(  0.5, -0.5, -0.5 );
    glTexCoord2f(1, 0);
    glVertex3f(  0.5,  0.5, -0.5 );
    glTexCoord2f(0, 0);
    glVertex3f( -0.5,  0.5, -0.5 );
    glTexCoord2f(0, 1);
    glVertex3f( -0.5, -0.5, -0.5 );
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Fixed point cloud

    srand48(123);
    glPointSize(5.0);
    glBegin(GL_POINTS);
    for (int i=0; i<100; i++) {
        glColor3f( 0, 1, 0 ); // drand48(), drand48(), drand48() );
        glVertex3f( 2.0*drand48()-1.0, 2.0*drand48()-1.0, 2.0*drand48()-1.0 );
        glColor3f(1,0,0);
        glVertex3f( 0, 0, 0 );
    }
    glEnd();

#endif

    CHECK_GL;

    return true;
}

const tinia::renderlist::DataBase*
APCJob::getRenderList( const std::string& session, const std::string& key )
{
    return &m_renderlist_db;
}
}
}
