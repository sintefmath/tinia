#include <iostream>
#include <tinia/renderlist/Logger.hpp>
#include <tinia/renderlist/Draw.hpp>
#include <tinia/renderlist/SetShader.hpp>
#include <tinia/renderlist/gl/RenderSetShader.hpp>
#include <tinia/renderlist/gl/RenderShader.hpp>
#include <tinia/renderlist/gl/Renderer.hpp>
#include "Utils.hpp"

namespace tinia {
namespace renderlist {
namespace gl {

static const std::string package = "renderlist.gl.RenderSetShader";

RenderSetShader::RenderSetShader( Renderer& renderer, Id id )
    : RenderAction( renderer, id ),
      m_gl_program( 0 )
{}


void
RenderSetShader::pull( const SetShader* a )
{
    static const std::string log = package + ".pull: ";

    const RenderShader* s = m_renderer.shader( a->shaderId() );
    if( s == NULL ) {
        std::cerr << "failed to locate shader id=" << a->shaderId() << std::endl;
        m_gl_program = 0;
    }
    else {
        m_gl_program = s->program();
    }
}

void
RenderSetShader::invoke( RenderState& state )
{
    glUseProgram( m_gl_program );
#ifdef DEBUG
    Logger log = getLogger( package + ".invoke" );
    CHECK_GL;
#endif
}




} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
