#pragma once

/** [headers]*/
#include <tinia/tinia.hpp>
#include <GL/glew.h>
/** [headers]*/

namespace tinia { namespace tutorial {


/** [class] */
class Tutorial4Job : public tinia::jobcontroller::OpenGLJob {
public:
    Tutorial4Job();

    bool renderFrame( const std::string &session,
                      const std::string &key,
                      unsigned int fbo,
                      const size_t width,
                      const size_t height );
/** [renderlistdecl] */
    renderlist::DataBase* getRenderList(const std::string &session,
                                        const std::string &key);
    /** [renderlistdecl] */

    /** [initGL] */
    bool initGL() { glewInit(); return true; }
    /** [initGL] */

private:
    /** [mdatabase] */
    tinia::renderlist::DataBase m_database;
    /** [mdatabase] */
};
/** [class]*/

/** [ctor] */
Tutorial4Job::Tutorial4Job()
{
    m_model->addElement( "myViewer", tinia::model::Viewer() );
    m_model->addElement("boundingbox", "0 0 0 1 1 1");

    /** [layout] */
    auto layout = new tinia::model::gui::VerticalLayout();
    /** [layout] */

    /** [canvas] */
    auto canvas = new tinia::model::gui::Canvas("myViewer");
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

    /** [buffer] */
    float vertices[] = {
                         0, 0, 0.5,
                         1, 0, 0.5,
                         1, 1, 0.5
                        };
    m_database.createBuffer("vertices")->set(vertices, 3*3);
    /** [buffer] */

    /** [shader] */
    std::string vertexShader =
            "uniform mat4 MVP;\n"
            "attribute vec3 position;\n"
            "void\n"
            "main()\n"
            "{\n"
            "    gl_Position = MVP * vec4( position, 1.0 );\n"
            "}\n";
    std::string fragmentShader =
            "#ifdef GL_ES\n"
            "precision highp float;\n"
            "#endif\n"
            "void\n"
            "main()\n"
            "{\n"
            "    gl_FragColor = vec4( 1,0,1, 1.0 );\n"
            "}\n";
    auto shader = m_database.createShader("shader");
    shader->setVertexStage(vertexShader);
    shader->setFragmentStage(fragmentShader);
    /** [shader] */

    /** [actionShader] */
    m_database.createAction<tinia::renderlist::SetShader>("useShader")
            ->setShader("shader");
    /** [actionShader] */

    /** [actionBuffer] */
    m_database.createAction<tinia::renderlist::SetInputs>("useBuffer")
            ->setShader("shader")
            ->setInput("position", "vertices", 3);
    /** [actionBuffer] */

    /** [actionMVP] */
    m_database.createAction<tinia::renderlist::SetUniforms>("setUniforms")
            ->setShader("shader")
            ->setSemantic("MVP", tinia::renderlist::SEMANTIC_MODELVIEW_PROJECTION_MATRIX);
    /** [actionMVP] */

    /** [actionDraw] */
    m_database.createAction<tinia::renderlist::Draw>("draw")
            ->setNonIndexed(tinia::renderlist::PRIMITIVE_LINE_LOOP, 0, 3);
    /** [actionDraw] */

    /** [drawOrder] */
    m_database.drawOrderClear()
            ->drawOrderAdd("useShader")
            ->drawOrderAdd("useBuffer")
            ->drawOrderAdd("setUniforms")
            ->drawOrderAdd("draw");
    /** [drawOrder] */

    /** [process] */
    m_database.process();
    /** [process] */
}
/** [ctor]*/

/** [renderframe] */
bool Tutorial4Job::renderFrame( const std::string &session,
                                const std::string &key,
                                unsigned int fbo,
                                const size_t width,
                                const size_t height )
{
    /** [viewer] */
    tinia::model::Viewer viewer;
    m_model->getElementValue("myViewer", viewer);
    /** [viewer] */

    glClearColor(0, 0, 0 ,0 );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);

    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf( viewer.projectionMatrix.data() );
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf( viewer.modelviewMatrix.data() );
    glColor3f(1, 0, 0);
    /** [renderloop] */
    glBegin(GL_TRIANGLES);
    glColor3f(1, 0, 0);
    glVertex3f(0, 0,0.5);
    glVertex3f(1, 0, 0.5);
    glVertex3f(1, 1, 0.5);
    glEnd();
    /** [renderloop] */


    /** [return]*/
    return true;
    /** [return] */
}
/** [renderframe] */

/** [renderlistfunc] */
renderlist::DataBase *Tutorial4Job::getRenderList(const std::string &session, const std::string &key)
{
    return &m_database;
}
/** [renderlistfunc] */
} // of tutorial
} // of tinia
