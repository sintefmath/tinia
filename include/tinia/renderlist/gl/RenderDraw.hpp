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

class RenderDraw : public RenderAction
{
public:
    RenderDraw( Renderer& renderer, Id id );

    void
    pull( const Draw* a );

    void
    invoke( RenderState& state );

protected:
    GLuint  m_index_buffer;
    GLenum  m_index_type;
    GLsizei m_index_size;
    GLenum  m_mode;
    GLsizei m_first;
    GLsizei m_count;

};



} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
