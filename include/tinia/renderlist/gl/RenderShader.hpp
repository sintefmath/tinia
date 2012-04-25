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
