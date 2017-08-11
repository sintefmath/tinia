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

#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <tinia/renderlist/RenderList.hpp>
#include <tinia/renderlist/gl/Renderer.hpp>

namespace tinia {
namespace renderlist {
namespace gl {

class RenderBuffer : public RenderItem
{
public:
    RenderBuffer( Renderer& renderer, Id id );

    ~RenderBuffer();

    /** Pull data from buffer item. */
    void
    pull( const Buffer* b );

    GLuint
    buffer() const { return m_gl_name; }

    GLenum
    type() const { return m_gl_type; }

    GLsizei
    elementSize() const { return m_bytesize; }

protected:
    GLuint      m_gl_name;      ///< Buffer object name.
    GLenum      m_gl_type;      ///< GL_INT or GL_FLOAT.
    GLsizei     m_count;        ///< The number of elements in the buffer.
    GLsizei     m_bytesize;     ///< Byte size of element.
};



} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
