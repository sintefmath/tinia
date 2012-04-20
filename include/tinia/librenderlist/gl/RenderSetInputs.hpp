#pragma once
#include <GL/glew.h>
#include <tinia/librenderlist/RenderList.hpp>
#include <tinia/librenderlist/gl/Renderer.hpp>

namespace librenderlist {
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
} // of namespace librenderlist
