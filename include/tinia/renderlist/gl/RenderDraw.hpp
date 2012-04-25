#pragma once
#include <GL/glew.h>
#include <tinia/renderlist/RenderList.hpp>
#include <tinia/renderlist/gl/Renderer.hpp>

namespace tinia {
namespace renderlist {
namespace gl {

class RenderDraw : public RenderAction
{
public:
    RenderDraw( Renderer& renderer, Id id );

    void
    pull( const Draw* a );

    void
    invoke( RenderState& state );

protected:
    GLuint  m_index_buffer;
    GLenum  m_index_type;
    GLsizei m_index_size;
    GLenum  m_mode;
    GLsizei m_first;
    GLsizei m_count;

};



} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
