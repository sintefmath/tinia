#include "siut2/dsrv/FreeglutWindow.h"

#include "siut2/dsrv/math/BBox.hpp"
#include "siut2/perf_utils/FpsCounter.hpp"

#include "siut2/gl_utils/GLSLtools.hpp"
#include "siut2/gl_utils/RenderText.hpp"
#include "siut2/gl_utils/RenderCoordAxis.hpp"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> //for c_ptr equvialent

#include <sstream>

#ifdef DEBUG_FGW
#define FREEGLUT_CONTEXT_FLAG GLUT_DEBUG
#else
#define FREEGLUT_CONTEXT_FLAG 0
#endif


#ifdef DEBUG_FGW
#define THROW_FGW
#include <stdexcept>
#endif

namespace siut2
{
namespace dsrv
{
    FreeglutWindow* FreeglutWindow::m_instance = NULL;

    FreeglutWindow::FreeglutWindow()
    : rescache_(NULL),
      m_viewer(NULL),
      m_fps_visible(false),
      m_coordsys_visible(false),
      m_context_set_up(false),
      m_fps_frames(0)
    {
      m_instance = this;
    }


void
FreeglutWindow::setUpMixedModeContext(int *argc, char **argv, const char *title, unsigned int display_mode)
{
    if(m_context_set_up)
    {
#ifdef DEBUG_FGW
        std::stringstream s;
        s <<  "Creating OpenGL contexts twice. This is not supported by DSRV!" << std::endl;
        throw std::runtime_error(s.str());
#endif
        return;
    }

    m_context_set_up = true;
    if(display_mode == 0)
    {
        display_mode = GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH;
    }

    glutInit(argc, argv);
    glutInitDisplayMode(display_mode);
    glutCreateWindow(title);
    glewExperimental = true;
    GLenum glew_err = glewInit();

#ifdef DEBUG_FGW
    if(glew_err != GLEW_OK)
    {
        std::stringstream s;
        s << "Whoops, glewInit, called from setUpMixedModeContext, failed which is not good. ";
        s << "The error was " << glewGetErrorString(glew_err) << std::endl;
        throw std::runtime_error(s.str());
    }

    if(display_mode & GLUT_MULTISAMPLE)
    {
        GLint buffers, samples;
        glGetIntegerv(GL_SAMPLE_BUFFERS, &buffers);
        glGetIntegerv(GL_SAMPLES, &samples);
        std::cerr << "Multisampling enabled with #" << buffers << " buffers and #" << samples << std::endl;
    }

    std::cout << "Using GLEW version: " << glewGetString(GLEW_VERSION) << std::endl;
#endif

    glutMouseFunc(mouseFunc);
    glutMotionFunc(motionFunc);
    glutReshapeFunc(reshapeFunc);
    glutDisplayFunc(displayMixedFunc);
    glutKeyboardFunc(keyboardFunc);
    glutIdleFunc(idleFunc);
    glutSpecialFunc(specialFunc);
    glutMouseWheelFunc(mouseWheelFunc);
    setTextColor(glm::vec3(1.f, 1.f, 1.f));


}
void
    FreeglutWindow::setUpContext(int *argc, char **argv, const char* title, unsigned int display_mode)
{
    if(!m_context_set_up)
    {
#ifdef DEBUG_FGW
        std::stringstream s;
        s <<  "Creating OpenGL contexts twice. This is not supported by DSRV!" << std::endl;
        throw std::runtime_error(s.str());
#endif
        return;
    }

    m_context_set_up = true;

    if(display_mode == 0)
    {
        display_mode = GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH;
    }

    glutInit(argc, argv);
    glutInitDisplayMode(display_mode);
    glutInitContextVersion(3,1);

    glutInitContextFlags(FREEGLUT_CONTEXT_FLAG | GLUT_FORWARD_COMPATIBLE);

    glutCreateWindow(title);

      // Required for access to VAO & similar in glew in 1.5.1
      glewExperimental = GL_TRUE;
      GLenum glew_err = glewInit();

#ifdef DEBUG_FGW
      if (glew_err != GLEW_OK)
    {
      std::stringstream s;
      s << "Whoops, glewInit, called from setUpContext, failed, that is *not* good. ";
      s << "The error was " << glewGetErrorString(glew_err) << std::endl;
      throw std::runtime_error(s.str());
    }
      if(display_mode & GLUT_MULTISAMPLE)
      {
          GLint buffers, samples;
          glGetIntegerv(GL_SAMPLE_BUFFERS, &buffers);
          glGetIntegerv(GL_SAMPLES, &samples);
          std::cerr << "Multisampling enabled with #" << buffers << " buffers and #" << samples << std::endl;
      }
      std::cout << "Using GLEW version: " << glewGetString(GLEW_VERSION) << std::endl;
#endif//end DEBUG

      glutMouseFunc(mouseFunc);
      glutMotionFunc(motionFunc);
      glutReshapeFunc(reshapeFunc);
      glutDisplayFunc(displayFunc);
      glutKeyboardFunc(keyboardFunc);
      glutIdleFunc(idleFunc);
      glutSpecialFunc(specialFunc);
      glutMouseWheelFunc(mouseWheelFunc);
      setTextColor(glm::vec3(1.f, 1.f, 1.f));
 }


 void
FreeglutWindow::setFpsVisibility(bool visible)
{
    m_fps_visible = visible;
}

 void
FreeglutWindow::setCoordSysVisibility( bool visible )
{
    m_coordsys_visible = visible;
}

 void
FreeglutWindow::init( glm::vec3 &min_, glm::vec3 &max_ )
{
     if(m_viewer != NULL)
    {
#ifdef DEBUG_FGW
    std::stringstream s;
    s << "trying to create multiple views, not supported yet. Happened in "\
                << __FILE__ << " at " << __LINE__ << std::endl;
    throw std::runtime_error(s.str());
#endif
    }
    m_viewer = new DSRViewer(min_, max_);
}

 void
FreeglutWindow::run()
{
    rescache_ = new gl_utils::ContextResourceCache();
    m_fps_omega.start();
    glutMainLoop();
}

 void
FreeglutWindow::setTextColor(const glm::vec3 &color)
{
    m_text_color = color;
}


 void
FreeglutWindow::mouseWheelFunc(int w, int d, int x, int y)
{
    m_instance->m_viewer->startMotion(DSRViewer::DOLLY, 0, 0);
    m_instance->m_viewer->motion(d * static_cast<int>(m_instance->m_viewer->getWindowSize()[0]) / 8, 0);
    m_instance->m_viewer->startMotion(DSRViewer::NONE, d, 0);
    glutPostRedisplay();
}

 void
FreeglutWindow::mouseFunc(int b, int s, int x, int y)
{
    if(s == GLUT_DOWN)
    {
    int modifier = glutGetModifiers();


    if(b == GLUT_LEFT_BUTTON) {
        if( modifier & GLUT_ACTIVE_SHIFT ) {
            m_instance->m_viewer->startMotion( DSRViewer::ORIENT, x, y );
        }
        else {
            m_instance->m_viewer->startMotion(DSRViewer::ROTATE, x, y);
        }
    }
    else if(b == GLUT_MIDDLE_BUTTON) {
        m_instance->m_viewer->startMotion(DSRViewer::PAN, x, y);
    }
    else if(b == GLUT_RIGHT_BUTTON) {
        if( modifier & GLUT_ACTIVE_SHIFT ) {
            m_instance->m_viewer->startMotion(DSRViewer::FOLLOW, x, y);
        }
        else {
            m_instance->m_viewer->startMotion(DSRViewer::ZOOM, x, y);
        }
    }
    }
    else {
    //m_instance->m_viewer->startMotion(DSRViewer::NONE, x, y);
        m_instance->m_viewer->endMotion(x, y);
    }
    glutPostRedisplay();
 }

 void
FreeglutWindow::motionFunc(int x, int y)
{
    m_instance->m_viewer->motion(x, y);
    glutPostRedisplay();
}


 void
FreeglutWindow::reshapeFunc( int w, int h )
{
    m_instance->m_viewer->setWindowSize( w, h );
    m_instance->reshape( w, h );
}


 void
FreeglutWindow::idleFunc()
{
    m_instance->idle();
    glutPostRedisplay();
}


 void
FreeglutWindow::keyboardFunc( unsigned char key, int x, int y )
{
    switch(key) {
    case 'q':
    case 'Q':
    case 27: //ESC
        glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
        glutLeaveMainLoop();
    break;
    default:
        m_instance->keyboard( key );
    }

    glutPostRedisplay();
}

void
FreeglutWindow::specialFunc( int key, int x, int y )
{
    m_instance->special( key, x, y );
    glutPostRedisplay();
}

void FreeglutWindow::displayMixedFunc()
{
    glm::vec2 wsize_temp = m_instance->m_viewer->getWindowSize();
    if( wsize_temp[0] > 0 && wsize_temp[1] > 0 ) {
        glViewport(0, 0, (GLsizei)wsize_temp[0], (GLsizei)wsize_temp[1]);
    }

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_instance->m_viewer->update();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(m_instance->m_viewer->getProjectionMatrix()));
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(m_instance->m_viewer->getModelviewMatrix()));

    m_instance->render(); //so that children get to render

    m_instance->m_fps_frames++;
    if(m_instance->m_fps_omega.elapsed() > 1.0)
    {
        static std::ostringstream os;
        os.flags(std::ios_base::right | std::ios_base::fixed);
        os.precision(1);
        double fps = static_cast<double>(m_instance->m_fps_frames)/m_instance->m_fps_omega.elapsed();
        os << "[ FPS: " << fps << "] " << m_instance->info();
        m_instance->m_fps_string = os.str();
        os.str("");
        m_instance->m_fps_frames = 0;
        m_instance->m_fps_omega.restart();
    }

    if(m_instance->m_fps_visible) {
        renderString( m_instance->rescache_, wsize_temp[0], wsize_temp[1], 8, 8, m_instance->m_fps_string  );
    }

    if( m_instance->m_coordsys_visible ) {
        glViewport(0, 0, 100, 100);
        glm::quat q_orientation = m_instance->m_viewer->getOrientation();
        glm::mat4 m4_orientation = glm::mat4_cast(q_orientation);
        renderCoordAxis(glm::value_ptr(m4_orientation), m_instance->rescache_);
        glViewport(0, 0, static_cast<GLsizei>(wsize_temp[0]), static_cast<GLsizei>(wsize_temp[1]));
    }
    glutSwapBuffers();
}

   void FreeglutWindow::displayFunc()
  {
      glm::vec2 wsize_temp = m_instance->m_viewer->getWindowSize();
      if( wsize_temp[0] > 0 && wsize_temp[1] > 0 ) {
          glViewport(0, 0, (GLsizei)wsize_temp[0], (GLsizei)wsize_temp[1]);
      }

      glEnable(GL_DEPTH_TEST);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      m_instance->m_viewer->update();

      m_instance->render(); //so that children get to render


      m_instance->m_fps_frames++;
      if(m_instance->m_fps_omega.elapsed() > 1.0)
      {
          static std::ostringstream os;
          os.flags(std::ios_base::right | std::ios_base::fixed);
          os.precision(1);

          double fps = static_cast<double>(m_instance->m_fps_frames)/m_instance->m_fps_omega.elapsed();
          os << "[ FPS: " << fps << "] " << m_instance->info();
          m_instance->m_fps_string = os.str();
          os.str("");
          m_instance->m_fps_frames = 0;
          m_instance->m_fps_omega.restart();
      }

      if(m_instance->m_fps_visible) {
          renderString( m_instance->rescache_, wsize_temp[0], wsize_temp[1], 8, 8, m_instance->m_fps_string  );
      }

      if( m_instance->m_coordsys_visible ) {
          glViewport(0, 0, 100, 100);
          glm::quat q_orientation = m_instance->m_viewer->getOrientation();
          glm::mat4 m4_orientation = glm::mat4_cast(q_orientation);
          renderCoordAxis(glm::value_ptr(m4_orientation), m_instance->rescache_);
          glViewport(0, 0, static_cast<GLsizei>(wsize_temp[0]), static_cast<GLsizei>(wsize_temp[1]));
      }
      glutSwapBuffers();

  }


   DSRViewer* FreeglutWindow::getDSRView(int i)
  {
    //needs to be updated when adding support for more views.
    return m_viewer;
  }


 void
FreeglutWindow::idle()
{
}

 void
FreeglutWindow::reshape(int w, int h)
{
}

 void
FreeglutWindow::keyboard(unsigned char key)
{
}

 void
FreeglutWindow::special( int key, int x, int y )
{
}

 std::string
FreeglutWindow::info()
{
    return std::string("");
}

 void
FreeglutWindow::render()
{
}

} // namespace dsrv
} //namespace siut2
