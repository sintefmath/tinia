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

    explicit WindowCoord( std::string& xy ) {
        std::istringstream buffer( xy );
        
        buffer >> x;
        buffer >> y;              
    }
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

    int textureSize;

    ImageCoord( const RelativeCoord& relativeCoord, const int textureSize ) 
        : textureSize( textureSize )
    {
        x = static_cast<int>( relativeCoord.x * textureSize );
        y = static_cast<int>( relativeCoord.y * textureSize );
    }

    int arrayPos() const {
        return y * textureSize + x;
    }
};

class TextureDrawer : public tinia::jobcontroller::OpenGLJob,
                      public tinia::model::StateListener
{
public:
    TextureDrawer();
    ~TextureDrawer();

    bool renderFrame( const std::string &session, const std::string &key,
                     unsigned int fbo, const size_t width, const size_t height );
    
    bool initGL();

    void stateElementModified ( tinia::model::StateElement *stateElement )  {        
        auto xy = m_model->getElementValue<std::string>( "click_xy" );    

        WindowCoord windowCoord( xy );      
        RelativeCoord relativeCoord( windowCoord,  m_model->getElementValue<tinia::model::Viewer> ( "myViewer" ) );
        ImageCoord imageCoord( relativeCoord, m_textureSize );
        
        colorPixel( imageCoord );
    }

private:
    void generateTexture();
    void colorPixel( const ImageCoord& imageCoord ) {
        auto pos = imageCoord.arrayPos();

        if ( validPos( pos ) ) {
            m_texData[pos] = glm::vec4( 1.0, 0.0, 0.0, 0.0 );
            uploadTexture();
        }
    }

    void uploadTexture() {
        glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, m_textureSize, m_textureSize,
            0, GL_RGBA, GL_FLOAT, m_texData.data() ); 
    }

    bool validPos( int pos ) {
        if ( pos >= 0 && pos < m_texData.size() ) {
            return true;
        } else {
            return false;
        }
    }

    GLuint m_texName;
    static const unsigned int m_textureSize = 256;
    std::array<glm::vec4, m_textureSize * m_textureSize> m_texData;       
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
    canvas->setViewerType( std::string("TextureDrawer") );
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
    
    /** [setupclicck] */
    m_model->addElement( "click_xy", "0 0");    
    m_model->addStateListener( "click_xy", this );

    /** [setupclicck] */       
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
    
    uploadTexture ();    
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