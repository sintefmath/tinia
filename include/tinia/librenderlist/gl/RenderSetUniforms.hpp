#pragma once
#include <GL/glew.h>
#include <tinia/librenderlist/RenderList.hpp>
#include <tinia/librenderlist/gl/Renderer.hpp>

namespace librenderlist {
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
} // of namespace librenderlist
