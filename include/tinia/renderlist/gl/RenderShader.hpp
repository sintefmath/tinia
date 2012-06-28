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

class RenderShader : public RenderItem
{
public:
    RenderShader( Renderer& renderer, Id id );

    ~RenderShader();

    void
    pull( const Shader* s );

    GLuint
    program() const { return m_gl_program; }

protected:
    GLuint  m_gl_program;

    bool
    compileShader( GLuint shader, const std::string& source );

    bool
    linkProgram( GLuint program );

};



} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
