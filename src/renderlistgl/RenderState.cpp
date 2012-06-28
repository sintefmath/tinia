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
#include <tinia/renderlist/gl/RenderState.hpp>

namespace tinia {
namespace renderlist {
namespace gl {

static const std::string package = "renderlist.gl.RenderState";

void
RenderState::setView( const float* projection,
                      const float* projection_inverse,
                      const float* view_from_world,
                      const float* view_to_world )
{
    m_projection = mat4FromPointer( projection );
    m_projection_inverse = mat4FromPointer( projection_inverse );
    m_view_from_world = mat4FromPointer( view_from_world );
    m_view_to_world = mat4FromPointer( view_to_world );
    update();
}

void
RenderState::setLocal( const float* local_from_world,
                       const float* local_to_world )
{
    m_local_from_world = mat4FromPointer( local_from_world );
    m_local_to_world = mat4FromPointer( local_to_world );
    update();
}

void
RenderState::update()
{
    m_modelview = m_view_from_world * m_local_to_world;
    m_modelview_inverse = m_local_from_world * m_view_to_world;
    m_modelview_projection = m_projection * m_modelview;
    m_normal_matrix = mat3FromMat4Transpose( m_modelview_inverse );
}

const glm::mat3
RenderState::mat3FromMat4Transpose( const glm::mat4& M )
{
    const float* P = glm::value_ptr( M );
    return glm::mat3( P[0], P[4], P[8],
                      P[1], P[5], P[9],
                      P[2], P[6], P[10] );

}

const glm::mat4
RenderState::mat4FromPointer( const float* M )
{
    return glm::mat4( M[0], M[1],  M[2], M[3],
                      M[4], M[5],  M[6], M[7],
                      M[8], M[9], M[10], M[11],
                      M[12], M[13], M[14], M[15] );
}


} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
