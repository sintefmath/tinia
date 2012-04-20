#pragma once
#include <GL/glew.h>
#include <tinia/librenderlist/RenderList.hpp>
#include <tinia/librenderlist/gl/Renderer.hpp>

namespace librenderlist {
namespace gl {

class RenderBuffer : public RenderItem
{
public:
    RenderBuffer( Renderer& renderer, Id id );

    ~RenderBuffer();

    /** Pull data from buffer item. */
    void
    pull( const Buffer* b );

    GLuint
    buffer() const { return m_gl_name; }

    GLenum
    type() const { return m_gl_type; }

    GLsizei
    elementSize() const { return m_bytesize; }

protected:
    GLuint      m_gl_name;      ///< Buffer object name.
    GLenum      m_gl_type;      ///< GL_INT or GL_FLOAT.
    GLsizei     m_count;        ///< The number of elements in the buffer.
    GLsizei     m_bytesize;     ///< Byte size of element.
};



} // of namespace gl
} // of namespace librenderlist
