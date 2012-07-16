#pragma once

/** [headers]*/
#include <tinia/tinia.hpp>
#include <GL/glew.h>
/** [headers]*/

namespace tinia { namespace tutorial {

/** [listener] */
/**[listenerdef]*/
class Tutorial5Listener : public tinia::model::StateListener {
/**[listenerdef]*/
public:
    // We need the model to update the boundingbox
    /**[listenerctor]*/
    Tutorial5Listener(std::shared_ptr<tinia::model::ExposedModel> model)
        : m_model(model)
    {
        /**[listenerctor]*/
        /**[addlistener]*/
        m_model->addStateListener("s1", this);
        m_model->addStateListener("s2", this);
        m_model->addStateListener("s3", this);
        /**[addlistener]*/
    }

    /** [removelistener]*/
    ~Tutorial5Listener()
    {
        m_model->removeStateListener("s1", this);
        m_model->removeStateListener("s2", this);
        m_model->removeStateListener("s3", this);

    }
    /** [removelistener]*/

    /** [stateelementmodified]*/
    void stateElementModified(tinia::model::StateElement *stateElement) {
        // Get the three values:
        int s1 = m_model->getElementValue<int>("s1");
        int s2 = m_model->getElementValue<int>("s2");
        int s3 = m_model->getElementValue<int>("s3");

        m_model->updateElement("boundingbox",
                               tinia::model::makeBoundingBoxString(1 - s1, 0,      0,
                                                                1 + s2, 1 + s3, 0));
    }
     /** [stateelementmodified]*/

private:
    std::shared_ptr<tinia::model::ExposedModel> m_model;
};
/** [listener] */


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
    /** [mlistener] */
    std::unique_ptr<Tutorial5Listener> m_listener;
    /** [mlistener] */
};
/** [class]*/

/** [ctor] */
Tutorial5Job::Tutorial5Job()
{
    m_model->addElement( "myViewer", tinia::model::Viewer() );
    m_model->addElement("boundingbox", "0 0 0 1 1 1");

    /** [constrained] */
    m_model->addConstrainedElement("s1", 0, 0, 10);
    m_model->addConstrainedElement("s2", 0, 0, 10);
    m_model->addConstrainedElement("s3", 0, 0, 10);
    /** [constrained] */

    /** [annotation] */
    m_model->addAnnotation("s1", "Left corner");
    m_model->addAnnotation("s2", "Right corner");
    m_model->addAnnotation("s3", "Upper corner");
    /** [annotation] */

    /** [label] */
    auto label1 = new tinia::model::gui::Label("s1");
    auto label2 = new tinia::model::gui::Label("s2");
    auto label3 = new tinia::model::gui::Label("s3");
    /** [label] */

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

    /**[slider]*/
    auto slider1 = new tinia::model::gui::HorizontalSlider("s1");
    auto slider2 = new tinia::model::gui::HorizontalSlider("s2");
    auto slider3 = new tinia::model::gui::HorizontalSlider("s3");
    /**[slider]*/

    /** [grid] */
    auto grid = new tinia::model::gui::Grid(3, 3);
    grid->setChild(0, 0, label1);
    grid->setChild(0, 1, slider1);
    grid->setChild(0, 2, new tinia::model::gui::HorizontalExpandingSpace());

    grid->setChild(1, 0, label2);
    grid->setChild(1, 1, slider2);
    grid->setChild(0, 2, new tinia::model::gui::HorizontalExpandingSpace());

    grid->setChild(2, 0, label3);
    grid->setChild(2, 1, slider3);
    grid->setChild(0, 2, new tinia::model::gui::HorizontalExpandingSpace());

    layout->addChild(grid);
    /** [grid] */

    /** [setgui] */
    m_model->setGUILayout(layout, tinia::model::gui::ALL);
    /** [setgui] */

    /** [clistener] */
    m_listener.reset(new Tutorial5Listener(m_model));
    /** [clistener] */
}
/** [ctor]*/

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

    /** [getscalars] */
    int s1 = m_model->getElementValue<int>("s1");
    int s2 = m_model->getElementValue<int>("s2");
    int s3 = m_model->getElementValue<int>("s3");
    /** [getscalars]*/


    glClearColor(0, 0, 0 ,0 );
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, width, height);

    /** [renderloop] */
    glBegin(GL_TRIANGLES);
    glColor3f(1, 0, 0);
    glVertex2f(0 - s1, 0);
    glVertex2f(1 + s2, 0);
    glVertex2f(1 + s2, 1 + s3);
    glEnd();
    /** [renderloop] */

    /** [return]*/
    return true;
    /** [return] */
}
/** [renderframe] */
} // of tutorial
} // of tinia
