#pragma once

/** [headers]*/
#include <tinia/tinia.hpp>
#include <GL/glew.h>
/** [headers]*/

namespace tinia { namespace tutorial {

/** [class] */
class Tutorial5Job : public tinia::jobcontroller::OpenGLJob {
public:
    Tutorial5Job();

    bool renderFrame( const std::string &session,
                      const std::string &key,
                      unsigned int fbo,
                      const size_t width,
                      const size_t height );
private:

};
/** [class]*/

/** [ctor] */
Tutorial5Job::Tutorial5Job()
{
    m_model->addElement( "myViewer", tinia::model::Viewer() );
    m_model->addElement("boundingbox", "-2 -2 -2 2 2 2");

    /** [layout] */
    auto layout = new tinia::model::gui::VerticalLayout();
    /** [layout] */

    /** [canvas] */
    auto canvas = new tinia::model::gui::Canvas("myViewer");
    canvas->setViewerType( std::string("FPSViewer") );
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

void DrawCube(float size) {

 glBegin(GL_QUADS);

  glColor3f(0.7, 0.0, 0.0);
  glVertex3f(-size, -size, -size);
  glVertex3f( size, -size, -size);
  glVertex3f( size,  size, -size);
  glVertex3f(-size,  size, -size);
  glColor3f(0.0, 1.0, 1.0);
  glVertex3f(-size, -size,  size);
  glVertex3f( size, -size,  size);
  glVertex3f( size,  size,  size);
  glVertex3f(-size,  size,  size);

  glColor3f(0.0, 0.0, 0.7);

  glVertex3f(-size, -size, -size);
  glVertex3f(-size, -size,  size);
  glVertex3f(-size,  size,  size);
  glVertex3f(-size,  size, -size);

  glColor3f( 1.0f, 0.25f, 0.0f);
  glVertex3f( size, -size, -size);
  glVertex3f( size, -size,  size);
  glVertex3f( size,  size,  size);
  glVertex3f( size,  size, -size);

  glColor3f(0.0, 0.7, 0.0);

  glVertex3f(-size, -size, -size);
  glVertex3f(-size, -size,  size);
  glVertex3f( size, -size,  size);
  glVertex3f( size, -size, -size);

  glColor3f( 0.25, 0.25, 0.25 );
  glVertex3f(-size, size, -size);
  glVertex3f(-size, size,  size);
  glVertex3f( size, size,  size);
  glVertex3f( size, size, -size);

 glEnd(); }

/** [renderframe] */
bool Tutorial5Job::renderFrame( const std::string &session,
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
