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

#include <tinia/renderlist/Logger.hpp>
#include <tinia/renderlist/gl/RenderBuffer.hpp>
#include <tinia/renderlist/Buffer.hpp>
#include "Utils.hpp"

namespace tinia {
namespace renderlist {
namespace gl {

static const std::string package = "renderlist.gl.RenderBuffer";

RenderBuffer::RenderBuffer( Renderer& renderer, Id id )
    : RenderItem( renderer, id ),
      m_gl_name( 0u ),
      m_gl_type( GL_FLOAT ),
      m_count( 0u ),
      m_bytesize( 0u )
{
    glGenBuffers( 1, &m_gl_name );
}

RenderBuffer::~RenderBuffer()
{
    glDeleteBuffers( 1, &m_gl_name );
}

void
RenderBuffer::pull( const Buffer* src )
{
    Logger log = getLogger( package + ".pull" );

    m_count = src->count();
    glBindBuffer( GL_ARRAY_BUFFER, m_gl_name );
    switch( src->type() ) {
    case ELEMENT_INT:
        m_gl_type = GL_INT;
        m_bytesize = sizeof(GLint);
        glBufferData( GL_ARRAY_BUFFER,
                      m_bytesize*m_count,
                      src->intData(),
                      GL_STATIC_DRAW );
        break;
    case ELEMENT_FLOAT:
        m_gl_type = GL_FLOAT;
        m_bytesize = sizeof(GLfloat);
        glBufferData( GL_ARRAY_BUFFER,
                      m_bytesize*m_count,
                      src->floatData(),
                      GL_STATIC_DRAW );
        break;
    }
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    CHECK_GL;
    RL_LOG_TRACE( log, "uploaded buffer (id=" << src->id() <<
                 ", name='" << src->name() << "')" <<
                 ", count=" << m_count <<
                 ", bytesize=" << m_bytesize );
}




} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
