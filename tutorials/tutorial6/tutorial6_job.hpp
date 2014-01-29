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

/** [headers]*/
#include <tinia/tinia.hpp>
#include <GL/glew.h>
/** [headers]*/

namespace tinia { namespace tutorial {

/** [class] */
class tutorial6 : public tinia::jobcontroller::OpenGLJob {
public:
    tutorial6();

    bool renderFrame( const std::string &session,
                      const std::string &key,
                      unsigned int fbo,
                      const size_t width,
                      const size_t height );
private:

};
/** [class]*/

/** [ctor] */
tutorial6::tutorial6()
{
    m_model->addElement( "myViewer", tinia::model::Viewer() );
    m_model->addElement("boundingbox", "-2 -2 -2 2 2 2");

    /** [layout] */
    tinia::model::gui::VerticalLayout* layout = new tinia::model::gui::VerticalLayout();
    /** [layout] */

    /** [canvas] */
    tinia::model::gui::Canvas* canvas = new tinia::model::gui::Canvas("myViewer");
    canvas->setViewerType( std::string("tutorial6") );
    /** [canvas] */

    /** [boundingbox] */
    canvas->boundingBoxKey("boundingbox");
    /** [boundingbox] */

    /** [add] */
    layout->addChild(canvas);
    /** [add] */



    /** [setgui] */
    m_model->setGUILayout(layout, tinia::model::gui::ALL);
    /** [setgui] */

}
/** [ctor]*/

/** [drawColoredBox] */
void DrawCube(float size) {

 glBegin(GL_QUADS);

  glColor3f(0.7f, 0.0f, 0.0f);
  glVertex3f(-size, -size, -size); glVertex3f( size, -size, -size); glVertex3f( size,  size, -size); glVertex3f(-size,  size, -size);
  glColor3f(0.0f, 1.0f, 1.0f);
  glVertex3f(-size, -size,  size); glVertex3f( size, -size,  size); glVertex3f( size,  size,  size); glVertex3f(-size,  size,  size);

  glColor3f(0.0f, 0.0f, 0.7f);
  glVertex3f(-size, -size, -size); glVertex3f(-size, -size,  size); glVertex3f(-size,  size,  size); glVertex3f(-size,  size, -size);

  glColor3f( 1.0f, 0.25f, 0.0f); 
  glVertex3f( size, -size, -size); glVertex3f( size, -size,  size); glVertex3f( size,  size,  size); glVertex3f( size,  size, -size);

  glColor3f(0.0f, 0.7f, 0.0f);
  glVertex3f(-size, -size, -size); glVertex3f(-size, -size,  size); glVertex3f( size, -size,  size); glVertex3f( size, -size, -size);

  glColor3f( 0.25f, 0.25f, 0.25f );
  glVertex3f(-size, size, -size); glVertex3f(-size, size,  size); glVertex3f( size, size,  size); glVertex3f( size, size, -size);

 glEnd(); 
}
/** [drawColoredBox] */

/** [renderframe] */
bool tutorial6::renderFrame( const std::string &session,
                                const std::string &key,
                                unsigned int fbo,
                                const size_t width,
                                const size_t height )
{
    /** [viewer] */
    tinia::model::Viewer viewer;
    m_model->getElementValue("myViewer", viewer);
    /** [viewer] */
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClear( GL_DEPTH_BUFFER_BIT );
    /** [matrices] */
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(viewer.modelviewMatrix.data());

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(viewer.projectionMatrix.data());
    /** [matrices] */


    glClearColor(0, 0, 0 ,0 );
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, width, height);

    /** [renderloop] */
    glShadeModel(GL_SMOOTH);

    DrawCube( 0.5f );

    /** [renderloop] */

    /** [return]*/
    return true;
    /** [return] */
}
/** [renderframe] */
} // of tutorial
} // of tinia
