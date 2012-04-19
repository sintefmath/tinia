#pragma once
#include <GL/glew.h>
#include <librenderlist/RenderList.hpp>
#include <librenderlist/gl/Renderer.hpp>

namespace librenderlist {
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
} // of namespace librenderlist
