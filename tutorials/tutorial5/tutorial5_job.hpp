#pragma once

#include <tinia/tinia.hpp>
#include <GL/glew.h>

namespace tinia { namespace tutorial5 {

class TextureDrawer : public tinia::jobcontroller::OpenGLJob {
public:
    TextureDrawer();

    bool renderFrame( const std::string &session, const std::string &key,
                     unsigned int fbo, const size_t width, const size_t height );
};


/** [ctor] */
TextureDrawer::TextureDrawer()
{
    m_model->addElement( "myViewer", tinia::model::Viewer() );
    m_model->addElement("boundingbox", "-2 -2 -2 2 2 2");

    /** [layout] */
    auto layout = new tinia::model::gui::VerticalLayout();
    /** [layout] */

    /** [canvas] */
    auto canvas = new tinia::model::gui::Canvas("myViewer");
    //canvas->setViewerType( std::string("FPSViewer") );
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
bool TextureDrawer::renderFrame( const std::string &session,
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

    //DrawCube( 0.5f );

    /** [renderloop] */

    /** [return]*/
    return true;
    /** [return] */
}

}}