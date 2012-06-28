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
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace tinia {
namespace renderlist {
namespace gl {

class RenderState
{
public:

    void
    setView( const float* projection,
             const float* projection_inverse,
             const float* view_from_world,
             const float* view_to_world );

    void
    setLocal( const float* local_from_world,
              const float* local_to_world );

    const float*
    projection() const { return glm::value_ptr( m_projection ); }

    const float*
    modelviewProjection() const { return glm::value_ptr( m_modelview_projection ); }

    const float*
    normalMatrix() const { return glm::value_ptr( m_normal_matrix ); }


protected:
    glm::mat4   m_projection;
    glm::mat4   m_projection_inverse;
    glm::mat4   m_view_to_world;
    glm::mat4   m_view_from_world;
    glm::mat4   m_local_to_world;
    glm::mat4   m_local_from_world;

    glm::mat3   m_normal_matrix;
    glm::mat4   m_modelview;
    glm::mat4   m_modelview_inverse;
    glm::mat4   m_modelview_projection;

    const glm::mat4
    mat4FromPointer( const float* M );

    const glm::mat3
    mat3FromMat4Transpose( const glm::mat4& M );

    void
    update();

};


} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
