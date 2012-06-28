/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

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
