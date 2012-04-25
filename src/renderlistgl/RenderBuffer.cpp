#include <tinia/renderlist/Logger.hpp>
#include <tinia/renderlist/gl/RenderBuffer.hpp>
#include <tinia/renderlist/Buffer.hpp>
#include "Utils.hpp"

namespace tinia {
namespace renderlist {
namespace gl {

static const std::string package = "renderlist.gl.RenderBuffer";

RenderBuffer::RenderBuffer( Renderer& renderer, Id id )
    : RenderItem( renderer, id ),
      m_gl_name( 0u ),
      m_gl_type( GL_FLOAT ),
      m_count( 0u ),
      m_bytesize( 0u )
{
    glGenBuffers( 1, &m_gl_name );
}

RenderBuffer::~RenderBuffer()
{
    glDeleteBuffers( 1, &m_gl_name );
}

void
RenderBuffer::pull( const Buffer* src )
{
    Logger log = getLogger( package + ".pull" );

    m_count = src->count();
    glBindBuffer( GL_ARRAY_BUFFER, m_gl_name );
    switch( src->type() ) {
    case ELEMENT_INT:
        m_gl_type = GL_INT;
        m_bytesize = sizeof(GLint);
        glBufferData( GL_ARRAY_BUFFER,
                      m_bytesize*m_count,
                      src->intData(),
                      GL_STATIC_DRAW );
        break;
    case ELEMENT_FLOAT:
        m_gl_type = GL_FLOAT;
        m_bytesize = sizeof(GLfloat);
        glBufferData( GL_ARRAY_BUFFER,
                      m_bytesize*m_count,
                      src->floatData(),
                      GL_STATIC_DRAW );
        break;
    }
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    CHECK_GL;
    RL_LOG_TRACE( log, "uploaded buffer (id=" << src->id() <<
                 ", name='" << src->name() << "')" <<
                 ", count=" << m_count <<
                 ", bytesize=" << m_bytesize );
}




} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
