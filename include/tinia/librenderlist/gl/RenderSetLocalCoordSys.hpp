#pragma once
#include <GL/glew.h>
#include <algorithm>
#include <tinia/librenderlist/RenderList.hpp>
#include <tinia/librenderlist/SetLocalCoordSys.hpp>
#include <tinia/librenderlist/gl/Renderer.hpp>
#include <tinia/librenderlist/gl/RenderState.hpp>

namespace tinia {
namespace librenderlist {
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
} // of namespace librenderlist
} // of namespace tinia
