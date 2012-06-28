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
#include <GL/glew.h>
#include <tinia/renderlist/RenderList.hpp>
#include <tinia/renderlist/gl/Renderer.hpp>

namespace tinia {
namespace renderlist {
namespace gl {

class RenderSetUniforms : public RenderAction
{
public:
    RenderSetUniforms( Renderer& renderer, Id id );

    void
    pull( const SetUniforms* a );

    void
    invoke( RenderState& state );

protected:
    struct Uniform {
        GLint                       m_location;
        UniformType                 m_type;
        UniformSemantic             m_semantic;
        union {
            float                   m_float[16];
            int                     m_int[16];
        }                           m_payload;
    };
    std::vector<Uniform>            m_uniforms;

};

} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
