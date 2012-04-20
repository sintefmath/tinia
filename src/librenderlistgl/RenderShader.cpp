#include <tinia/librenderlist/Logger.hpp>
#include <tinia/librenderlist/gl/RenderShader.hpp>
#include <tinia/librenderlist/Shader.hpp>
#include "Utils.hpp"

namespace tinia {
namespace librenderlist {
namespace gl {

static const std::string package = "librenderlist.gl.RenderShader";


RenderShader::RenderShader(Renderer &renderer, Id id)
    : RenderItem( renderer, id ),
      m_gl_program( 0u )
{
    m_gl_program = glCreateProgram();
}

RenderShader::~RenderShader()
{
    glDeleteProgram( m_gl_program );
}

void
RenderShader::pull( const Shader* src )
{
    Logger log = getLogger( package + ".pull" );

    glDeleteProgram( m_gl_program );
    m_gl_program = glCreateProgram();

    if( !src->vertexStage().empty() ) {
        GLuint s = glCreateShader( GL_VERTEX_SHADER );
        compileShader( s, src->vertexStage() );
        glAttachShader( m_gl_program, s );
        glDeleteShader( s );
    }
    if( !src->tessCtrlStage().empty() ) {
        GLuint s = glCreateShader( GL_TESS_CONTROL_SHADER );
        compileShader( s, src->tessCtrlStage() );
        glAttachShader( m_gl_program, s );
        glDeleteShader( s );
    }
    if( !src->tessEvalStage().empty() ) {
        GLuint s = glCreateShader( GL_TESS_EVALUATION_SHADER );
        compileShader( s, src->tessEvalStage() );
        glAttachShader( m_gl_program, s );
        glDeleteShader( s );
    }
    if( !src->geometryStage().empty() ) {
        GLuint s = glCreateShader( GL_GEOMETRY_SHADER );
        compileShader( s, src->geometryStage() );
        glAttachShader( m_gl_program, s );
        glDeleteShader( s );
    }
    if( !src->fragmentStage().empty() ) {
        GLuint s = glCreateShader( GL_FRAGMENT_SHADER );
        compileShader( s, src->fragmentStage() );
        glAttachShader( m_gl_program, s );
        glDeleteShader( s );
    }
    linkProgram( m_gl_program );
    RL_LOG_TRACE( log, "built shader program (id=" << src->id() << ", name='" << src->name() << "')" );
    CHECK_GL;
}

bool
RenderShader::compileShader( GLuint shader, const std::string& source )
{
    Logger log = getLogger( package + ".compileShader" );

    const char* src = source.c_str();
    glShaderSource( shader, 1, &src, NULL );
    glCompileShader( shader );

    GLint status;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &status );

    GLint loglength;
    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &loglength );

    if( (status != GL_TRUE) || (loglength>1) ) {
        RL_LOG_ERROR( log, "shader source:" << std::endl << source );
    }
    if( loglength > 1 ) {
        std::vector<GLchar> clog( loglength );
        glGetShaderInfoLog( shader, loglength, NULL, clog.data() );
        RL_LOG_ERROR( log, "compilation log:" << std::endl << clog.data() );
    }
    if( status != GL_TRUE ) {
        RL_LOG_ERROR( log, "Compilation failed" );
    }
    CHECK_GL;
    return status == GL_TRUE;
}

bool
RenderShader::linkProgram( GLuint program )
{
    Logger log = getLogger( package + ".linkProgram" );

    glLinkProgram( program );

    GLint loglength;
    glGetProgramiv( program, GL_INFO_LOG_LENGTH, &loglength );
    if( loglength > 1 ) {
        std::vector<GLchar> clog( loglength );
        glGetProgramInfoLog( program, loglength, NULL, clog.data() );
        RL_LOG_ERROR( log, "Link log: " << std::endl << std::string( clog.data() ) );
    }

    GLint status;
    glGetProgramiv( program, GL_LINK_STATUS, &status );
    if( status != GL_TRUE ) {
        RL_LOG_ERROR( log, "Linking failed" );
    }

    CHECK_GL;
    return status == GL_TRUE;
}


} // of namespace gl
} // of namespace librenderlist
} // of namespace tinia
