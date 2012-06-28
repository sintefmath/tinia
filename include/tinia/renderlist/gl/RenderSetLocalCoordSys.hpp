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
#include <algorithm>
#include <tinia/renderlist/RenderList.hpp>
#include <tinia/renderlist/SetLocalCoordSys.hpp>
#include <tinia/renderlist/gl/Renderer.hpp>
#include <tinia/renderlist/gl/RenderState.hpp>

namespace tinia {
namespace renderlist {
namespace gl {

class RenderSetLocalCoordSys : public RenderAction
{
public:
    RenderSetLocalCoordSys( Renderer& renderer, Id id )
        : RenderAction( renderer, id )
    {}

    void
    pull( const SetLocalCoordSys* a )
    {
        std::copy_n( a->fromWorld(), 16, m_from_world );
        std::copy_n( a->toWorld(), 16, m_to_world );
    }

    void
    invoke( RenderState& state )
    {
        state.setLocal( m_from_world, m_to_world );
    }

protected:
    float   m_from_world[16];
    float   m_to_world[16];
};


} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
