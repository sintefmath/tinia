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
class Tutorial2Job : public tinia::jobcontroller::OpenGLJob {
public:
    Tutorial2Job();

    bool renderFrame( const std::string &session,
                      const std::string &key,
                      unsigned int fbo,
                      const size_t width,
                      const size_t height );
};
/** [class]*/

/** [ctor] */
Tutorial2Job::Tutorial2Job()
{
    m_model->addElement( "myViewer", tinia::model::Viewer() );
    m_model->addElement("boundingbox", "0 0 0 1 1 1");

    /** [layout] */
    tinia::model::gui::VerticalLayout* layout = new tinia::model::gui::VerticalLayout();
    /** [layout] */

    /** [canvas] */
    tinia::model::gui::Canvas* canvas = new tinia::model::gui::Canvas("myViewer");
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

/** [renderframe] */
bool Tutorial2Job::renderFrame( const std::string &session,
                                const std::string &key,
                                unsigned int fbo,
                                const size_t width,
                                const size_t height )
{
    /** [viewer] */
    tinia::model::Viewer viewer;
    m_model->getElementValue("myViewer", viewer);
    /** [viewer] */

    /** [matrices] */
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(viewer.modelviewMatrix.data());

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(viewer.projectionMatrix.data());
    /** [matrices] */

    /** [renderloop] */
    glClearColor(0, 0, 0 ,0 );
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, width, height);

    glBegin(GL_TRIANGLES);
    glColor3f(1, 0, 0);
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    glVertex2f(1, 1);
    glEnd();
    /** [renderloop] */

    /** [return]*/
    return true;
    /** [return] */
}
/** [renderframe] */
} // of tutorial
} // of tinia
