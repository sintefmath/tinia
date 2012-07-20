#pragma once

#include <tinia/tinia.hpp>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <array>
#include <algorithm>
#include <tuple>

namespace tinia { namespace tutorial5 {


struct WindowCoord {
    int x, y;

    /** [WindowCoordCtor] */
    explicit WindowCoord( std::string& xy ) {
        std::istringstream buffer( xy );
        
        buffer >> x;
        buffer >> y;              
    }
    /** [WindowCoordCtor] */
};

struct RelativeCoord {
    double x, y;

    RelativeCoord( const WindowCoord& windowCoord, const tinia::model::Viewer& viewer ) {
        x = windowCoord.x / static_cast<double>( viewer.width );
        y = windowCoord.y / static_cast<double>( viewer.height );
    };
};

struct ImageCoord {
    int x, y;    

    ImageCoord( const RelativeCoord& relativeCoord, const int textureSize )        
    {
        x = static_cast<int>( relativeCoord.x * textureSize );
        y = static_cast<int>( relativeCoord.y * textureSize );
    }    
};

/** [declaration] */
class TextureDrawer : public tinia::jobcontroller::OpenGLJob,
                      public tinia::model::StateListener
/** [declaration] */
{
public:
    TextureDrawer();
    ~TextureDrawer();

/** [overrides] */
    bool renderFrame( const std::string &session, const std::string &key,
                     unsigned int fbo, const size_t width, const size_t height );
    
    bool initGL();
    void stateElementModified ( tinia::model::StateElement *stateElement );
/** [overrides] */
private:
    void generateTexture();
    void colorPixel( const ImageCoord& imageCoord );    
  
    GLuint m_texName;
    static const unsigned int m_textureSize = 64;
};

/** [ctor] */
TextureDrawer::TextureDrawer() {    
    m_model->addElement( "myViewer", tinia::model::Viewer() );
    m_model->addElement("boundingbox", "-2 -2 -2 2 2 2");

    /** [layout] */
    auto layout = new tinia::model::gui::VerticalLayout();
    /** [layout] */

    /** [canvas] */
    auto canvas = new tinia::model::gui::Canvas("myViewer");
    canvas->setViewerType( std::string("MouseClickResponder") );
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
    
    /** [setupclick] */
    m_model->addElement( "click_xy", "0 0");    
    m_model->addStateListener( "click_xy", this );
    /** [setupclick] */       
}
/** [ctor]*/

TextureDrawer::~TextureDrawer() {
    glDeleteTextures ( 1, &m_texName );

}

/** [initGL] */
bool TextureDrawer::initGL () {
    glewInit();
    generateTexture ();

    return true;
}
/** [initGL] */

/** [stateElementModified] */
void TextureDrawer::stateElementModified ( tinia::model::StateElement *stateElement ) {
    auto xy = m_model->getElementValue<std::string>( "click_xy" );    

    WindowCoord windowCoord( xy );      
    RelativeCoord relativeCoord( windowCoord,  
        m_model->getElementValue<tinia::model::Viewer> ( "myViewer" ) );
    ImageCoord imageCoord( relativeCoord, m_textureSize );

    colorPixel( imageCoord );
}
/** [stateElementModified] */


void TextureDrawer::generateTexture() {
    std::array<glm::vec4, m_textureSize * m_textureSize> texData;
    glm::vec4 defaultColor( 0x88/256., 0xbf/256., 0xdb/256., 1.0 );

    std::fill( std::begin( texData ), std::end( texData ), defaultColor );

    glGenTextures ( 1, &m_texName );
    glBindTexture ( GL_TEXTURE_2D, m_texName );    

    // Mandatory on Nvidia-hardware
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, m_textureSize, m_textureSize,
        0, GL_RGBA, GL_FLOAT, texData.data() );     
}

/** [colorPixel] */
void TextureDrawer::colorPixel( const ImageCoord& imageCoord ) {
    glm::vec4 drawColor( 1.0, 1.0, 1.0, 1.0 );
    
    glTexSubImage2D ( GL_TEXTURE_2D, 0,
                     imageCoord.x, imageCoord.y, 1, 1, 
                     GL_RGBA, GL_FLOAT, &drawColor );
}
/** [colorPixel] */


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
    glEnable( GL_TEXTURE_2D );    
    glBindTexture( GL_TEXTURE_2D, m_texName );
    
    glBegin( GL_QUADS );        
        glTexCoord2f ( 0.f, 1.f );
        glVertex2f( -1.f, -1.f );
        
        glTexCoord2f( 1.f, 1.f );
        glVertex2f(  1.f, -1.f );
        
        glTexCoord2f ( 1.f, 0.f); 
        glVertex2f(  1.f,  1.f );
        
        glTexCoord2f( 0.f, 0.f ); 
        glVertex2f( -1.f,  1.f );
    glEnd();

    glDisable( GL_TEXTURE_2D );    
    /** [renderloop] */


    /** [return]*/
    return true;
    /** [return] */
}

}}
