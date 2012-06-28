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
#include <string>
#include <tinia/renderlist/Logger.hpp>
#include <tinia/renderlist/SetInputs.hpp>
#include <tinia/renderlist/gl/RenderSetInputs.hpp>
#include <tinia/renderlist/gl/RenderShader.hpp>
#include <tinia/renderlist/gl/RenderBuffer.hpp>
#include "Utils.hpp"

namespace tinia {
namespace renderlist {
namespace gl {

static const std::string package = "renderlist.gl.RenderSetInputs";


RenderSetInputs::RenderSetInputs( Renderer& renderer, Id id )
    : RenderAction( renderer, id ),
      m_vertex_array( 0 )
{
}

RenderSetInputs::~RenderSetInputs()
{
    if( m_vertex_array != 0 ) {
        glDeleteVertexArrays( 1, &m_vertex_array );
    }
}

void
RenderSetInputs::pull( const SetInputs* a )
{
    Logger log = getLogger( package + ".pull" );

    if( m_vertex_array != 0 ) {
        glDeleteVertexArrays( 1, &m_vertex_array );
        m_vertex_array = 0;
    }

    const RenderShader* s = m_renderer.shader( a->shaderId() );
    if( s == NULL ) {
        RL_LOG_ERROR( log, "Unknown shader (id=" << a->shaderId() << ")" );
        return;
    }
    std::vector<const RenderBuffer*> b( a->count() );
    for( size_t i=0; i<a->count(); i++ ) {
        b[i] = m_renderer.buffer( a->bufferId(i) );
        if( b[i] == NULL ) {
            RL_LOG_ERROR( log , "Unknown buffer (id=" << a->bufferId(i) << ")" );
            return;
        }
    }

    glGenVertexArrays( 1, &m_vertex_array );

    glBindVertexArray( m_vertex_array );
    for( size_t i=0; i<a->count(); i++ ) {
        GLint index = glGetAttribLocation( s->program(), a->symbol(i).c_str() );
        if( index == -1 ) {
            RL_LOG_WARN( log, "couldn't find symbol '" << a->symbol(i) << "'" );
        }
        else {
            glBindBuffer( GL_ARRAY_BUFFER, b[i]->buffer() );
            glVertexAttribPointer( index,
                                   a->components(i),
                                   b[i]->type(),
                                   GL_FALSE,
                                   b[i]->elementSize()*a->stride(i),
                                   reinterpret_cast<const GLvoid*>( b[i]->elementSize()*a->offset(i) ) );
            glEnableVertexAttribArray( index );
        }
    }
    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    CHECK_GL;
    RL_LOG_TRACE( log, "created shader inputs." );
}

void
RenderSetInputs::invoke( RenderState& state )
{
    glBindVertexArray( m_vertex_array );
#ifdef DEBUG
    Logger log = getLogger( package + ".invoke" );
    CHECK_GL;
#endif
}

} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
