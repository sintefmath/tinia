#pragma once
#include <GL/glew.h>
#include <tinia/renderlist/RenderList.hpp>
#include <tinia/renderlist/gl/Renderer.hpp>

namespace tinia {
namespace renderlist {
namespace gl {

class RenderSetInputs : public RenderAction
{
public:
    RenderSetInputs( Renderer& renderer, Id id );

    ~RenderSetInputs();

    void
    pull( const SetInputs* a );

    void
    invoke( RenderState& state );

protected:
    GLuint  m_vertex_array;
};



} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
