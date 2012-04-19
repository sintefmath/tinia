/* -*- mode: C++; tab-width:4; c-basic-offset: 4; indent-tabs-mode:nil -*- */
/************************************************************************
 *
 *  File: RenderText.hpp
 *
 *  Created: 2009-08-13
 *
 *  Version:
 *
 *  Authors: Christopher Dyken <christopher.dyken@sintef.no>
 *
 *  This file is part of the siut library.
 *  Copyright (C) 2009 by SINTEF.  All rights reserved.
 *
 *  SINTEF, Pb 124 Blindern, N-0314 Oslo, Norway
 *  http://www.sintef.no
 *********************************************************************/
#pragma once

#ifdef USE_GLEW
#include <GL/glew.h>
#else
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <string>
#include <vector>

#include "siut2/gl_utils/ContextResourceCache.hpp"
#include <glm/glm.hpp>


namespace siut2 {
  namespace gl_utils {

  // GL_C3F_V3F format.
    static GLfloat coordsys_geometry[3*20*6] =
    {
        // x-axis, red
        1.f, 0.f, 0.f,   0.05f,-0.05f, 0.05f,
        1.f, 0.f, 0.f,   1.00f,-0.05f, 0.05f,
        1.f, 0.f, 0.f,   1.00f, 0.05f, 0.05f,
        1.f, 0.f, 0.f,   0.05f, 0.05f, 0.05f,
        1.f, 0.f, 0.f,   0.05f, 0.05f, 0.05f,
        1.f, 0.f, 0.f,   1.00f, 0.05f, 0.05f,
        1.f, 0.f, 0.f,   1.00f, 0.05f,-0.05f,
        1.f, 0.f, 0.f,   0.05f, 0.05f,-0.05f,
        1.f, 0.f, 0.f,   0.05f, 0.05f,-0.05f,
        1.f, 0.f, 0.f,   1.00f, 0.05f,-0.05f,
        1.f, 0.f, 0.f,   1.00f,-0.05f,-0.05f,
        1.f, 0.f, 0.f,  -0.05f,-0.05f,-0.05f,
        1.f, 0.f, 0.f,  -0.05f,-0.05f,-0.05f,
        1.f, 0.f, 0.f,   1.00f,-0.05f,-0.05f,
        1.f, 0.f, 0.f,   1.00f,-0.05f, 0.05f,
        1.f, 0.f, 0.f,   0.05f,-0.05f, 0.05f,
        1.f, 0.f, 0.f,   1.00f,-0.05f, 0.05f,
        1.f, 0.f, 0.f,   1.00f,-0.05f,-0.05f,
        1.f, 0.f, 0.f,   1.00f, 0.05f,-0.05f,
        1.f, 0.f, 0.f,   1.00f, 0.05f, 0.05f,
        // y-axis, green
        0.f, 1.f, 0.f,  -0.05f,-0.05f,-0.05f,
        0.f, 1.f, 0.f,  -0.05f, 1.00f,-0.05f,
        0.f, 1.f, 0.f,   0.05f, 1.00f,-0.05f,
        0.f, 1.f, 0.f,   0.05f, 0.05f,-0.05f,
        0.f, 1.f, 0.f,   0.05f, 0.05f,-0.05f,
        0.f, 1.f, 0.f,   0.05f, 1.00f,-0.05f,
        0.f, 1.f, 0.f,   0.05f, 1.00f, 0.05f,
        0.f, 1.f, 0.f,   0.05f, 0.05f, 0.05f,
        0.f, 1.f, 0.f,   0.05f, 0.05f, 0.05f,
        0.f, 1.f, 0.f,   0.05f, 1.00f, 0.05f,
        0.f, 1.f, 0.f,  -0.05f, 1.00f, 0.05f,
        0.f, 1.f, 0.f,  -0.05f, 0.05f, 0.05f,
        0.f, 1.f, 0.f,  -0.05f, 0.05f, 0.05f,
        0.f, 1.f, 0.f,  -0.05f, 1.00f, 0.05f,
        0.f, 1.f, 0.f,  -0.05f, 1.00f,-0.05f,
        0.f, 1.f, 0.f,  -0.05f,-0.05f,-0.05f,
        0.f, 1.f, 0.f,  -0.05f, 1.00f,-0.05f,
        0.f, 1.f, 0.f,  -0.05f, 1.00f, 0.05f,
        0.f, 1.f, 0.f,   0.05f, 1.00f, 0.05f,
        0.f, 1.f, 0.f,   0.05f, 1.00f,-0.05f,
        // z-axis, blue
        0.f, 0.f, 1.f,  -0.05f, 0.05f, 0.05f,
        0.f, 0.f, 1.f,  -0.05f, 0.05f, 1.00f,
        0.f, 0.f, 1.f,   0.05f, 0.05f, 1.00f,
        0.f, 0.f, 1.f,   0.05f, 0.05f, 0.05f,
        0.f, 0.f, 1.f,   0.05f, 0.05f, 0.05f,
        0.f, 0.f, 1.f,   0.05f, 0.05f, 1.00f,
        0.f, 0.f, 1.f,   0.05f,-0.05f, 1.00f,
        0.f, 0.f, 1.f,   0.05f,-0.05f, 0.05f,
        0.f, 0.f, 1.f,   0.05f,-0.05f, 0.05f,
        0.f, 0.f, 1.f,   0.05f,-0.05f, 1.00f,
        0.f, 0.f, 1.f,  -0.05f,-0.05f, 1.00f,
        0.f, 0.f, 1.f,  -0.05f,-0.05f,-0.05f,
        0.f, 0.f, 1.f,  -0.05f,-0.05f,-0.05f,
        0.f, 0.f, 1.f,  -0.05f,-0.05f, 1.00f,
        0.f, 0.f, 1.f,  -0.05f, 0.05f, 1.00f,
        0.f, 0.f, 1.f,  -0.05f, 0.05f, 0.05f,
        0.f, 0.f, 1.f,  -0.05f, 0.05f, 1.00f,
        0.f, 0.f, 1.f,  -0.05f,-0.05f, 1.00f,
        0.f, 0.f, 1.f,   0.05f,-0.05f, 1.00f,
        0.f, 0.f, 1.f,   0.05f, 0.05f, 1.00f
    };

class RenderCoordSysResource
: public ContextResource
{
    friend void
    renderCoordSysSolid( ContextResourceCache* rc,
                         const glm::vec4& text_color );



public:

    /**
      *
      * \sideeffect Current texture unit. Texture unit 0.
      */
    RenderCoordSysResource()
    {
        glGenBuffers( 1, &m_vbo );
        glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
        glBufferData( GL_ARRAY_BUFFER,
                      3*20*6*sizeof(GLfloat),
                      &coordsys_geometry[0],
                      GL_STATIC_DRAW );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }

    ~RenderCoordSysResource()
    {
        glDeleteBuffers( 1, &m_vbo );
    }

protected:

    void
    render( const glm::vec4& text_color )
    {
        glUseProgram( 0 );
        glPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT );
        glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
        glInterleavedArrays( GL_C3F_V3F, 0, NULL );
        glDrawArrays( GL_QUADS, 0, 3*20 );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glPopClientAttrib();
    }

    GLuint m_vbo;
};


void
renderCoordSysSolid( ContextResourceCache* rc,
                     const glm::vec4& text_color = glm::vec4( 1.f, 1.f, 1.f, 1.f ) )
{
    if( rc == NULL ) {
        return;
    }

    RenderCoordSysResource* res =
            static_cast<RenderCoordSysResource*>( rc->getResource( ContextResourceCache::RESOURCE_RENDERCOORDSYS ) );
    if( res == NULL ) {
        res = new RenderCoordSysResource();
        rc->setResource( ContextResourceCache::RESOURCE_RENDERCOORDSYS, res );
    }
    res->render( text_color );
}




  } // of gl_utils
} // of siut
#endif // of _SIUT_GL_RENDERTEXT_HPP_
