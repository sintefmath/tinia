#pragma once

#include <tinia/tinia.hpp>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <array>
#include <algorithm>

namespace tinia { namespace tutorial5 {

class TextureDrawer : public tinia::jobcontroller::OpenGLJob {
public:
    TextureDrawer();
    ~TextureDrawer();

    bool renderFrame( const std::string &session, const std::string &key,
                     unsigned int fbo, const size_t width, const size_t height );
    
    bool initGL() override;

private:
    void generateTexture();
    GLuint m_texName;
    static const unsigned int m_textureSize = 256;
    std::array<glm::vec4, m_textureSize * m_textureSize> m_texData;
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

TextureDrawer::~TextureDrawer() {
    glDeleteTextures ( 1, &m_texName );

}

bool TextureDrawer::initGL () {
    glewInit();
    generateTexture ();

    return true;
}

void TextureDrawer::generateTexture() {
    glGenTextures ( 1, &m_texName );
    glBindTexture ( GL_TEXTURE_2D, m_texName );    
    
    std::fill( std::begin( m_texData ), std::end( m_texData ), glm::vec4( 0.5, 0.5, 0.7, 1.0 ) );
    
    // Mandatory on Nvidia-hardware
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, m_textureSize, m_textureSize,
                   0, GL_RGBA, GL_FLOAT, m_texData.data() );    
}

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
    //glShadeModel(GL_SMOOTH);

    glEnable( GL_TEXTURE_2D );
    //glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, m_texName );
    
    glBegin( GL_QUADS );
        glTexCoord2f ( 0.f, 0.f );
        glVertex2f( -1.f, -1.f );

        glTexCoord2f( 1.f, 0.f );
        glVertex2f(  1.f, -1.f );

        glTexCoord2f ( 1.f, 1.f);
        glVertex2f(  1.f,  1.f );

        glTexCoord2f( 0.f, 1.f );
        glVertex2f( -1.f,  1.f );
    glEnd();

    glDisable( GL_TEXTURE_2D );

    //DrawCube( 0.5f );

    /** [renderloop] */


    /** [return]*/
    return true;
    /** [return] */
}

}}