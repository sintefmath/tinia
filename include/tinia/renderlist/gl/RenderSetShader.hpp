#pragma once
#include <GL/glew.h>
#include <tinia/renderlist/RenderList.hpp>
#include <tinia/renderlist/gl/Renderer.hpp>

namespace tinia {
namespace renderlist {
namespace gl {

class RenderSetShader : public RenderAction
{
public:
    RenderSetShader( Renderer& renderer, Id id );

    void
    pull( const SetShader* a );

    void
    invoke( RenderState& state );

protected:
    GLuint  m_gl_program;
};



} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
