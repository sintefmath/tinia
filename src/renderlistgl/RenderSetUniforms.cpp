/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <tinia/renderlist/Logger.hpp>
#include <tinia/renderlist/SetUniforms.hpp>
#include <tinia/renderlist/gl/RenderShader.hpp>
#include <tinia/renderlist/gl/RenderSetUniforms.hpp>
#include <tinia/renderlist/gl/RenderState.hpp>
#include "Utils.hpp"

namespace tinia {
namespace renderlist {
namespace gl {
static const std::string package = "renderlist.gl.RenderSetUniforms";

RenderSetUniforms::RenderSetUniforms( Renderer& renderer, Id id )
    : RenderAction( renderer, id )
{}

void
RenderSetUniforms::pull( const SetUniforms* a )
{
    Logger log = getLogger( package + ".pull" );

    const RenderShader* s = m_renderer.shader( a->shaderId() );
    if( s == NULL ) {
        RL_LOG_ERROR( log, "Unknow shader, action id=" << a->id() << ", shader id=" << a->shaderId() );
        return;
    }

    m_uniforms.clear();
    for( size_t i=0; i<a->count(); i++ ) {
        GLint loc = glGetUniformLocation( s->program(), a->symbol(i).c_str() );
        if( loc == -1 ) {
            RL_LOG_DEBUG( log, "Couldn't find symbol " << a->symbol(i) << ", action id=" << a->id() );
        }
        else {
            m_uniforms.resize( m_uniforms.size()+1 );
            Uniform& u = m_uniforms.back();
            u.m_location = loc;
            u.m_type = a->type(i);
            u.m_semantic = a->semantic(i);
            switch( u.m_type ) {
            case UNIFORM_SEMANTIC:
                break;
            case UNIFORM_INT:
                std::copy_n( a->intData(i), 1, u.m_payload.m_int );
                break;
            case UNIFORM_FLOAT:
                std::copy_n( a->floatData(i), 1, u.m_payload.m_float );
                break;
            case UNIFORM_FLOAT2:
                std::copy_n( a->floatData(i), 2, u.m_payload.m_float );
                break;
            case UNIFORM_FLOAT3:
                std::copy_n( a->floatData(i), 3, u.m_payload.m_float );
                break;
            case UNIFORM_FLOAT4:
                std::copy_n( a->floatData(i), 4, u.m_payload.m_float );
                break;
            case UNIFORM_FLOAT3X3:
                std::copy_n( a->floatData(i), 9, u.m_payload.m_float );
                break;
            case UNIFORM_FLOAT4X4:
                std::copy_n( a->floatData(i), 16, u.m_payload.m_float );
                break;
            }
        }
    }

    CHECK_GL;
    RL_LOG_TRACE( log, "created set shader uniforms:" );
    for( size_t i=0; i<m_uniforms.size(); i++ ) {
        RL_LOG_TRACE( log, "  loc=" << m_uniforms[i].m_location <<
                     ", type=" << m_uniforms[i].m_type <<
                     ", sem=" << m_uniforms[i].m_semantic );
    }
}

void
RenderSetUniforms::invoke( RenderState& state )
{

    for(size_t i=0; i<m_uniforms.size(); i++ ) {
        UniformType type = m_uniforms[i].m_type;
        const void* data = m_uniforms[i].m_payload.m_float;
        if( type == UNIFORM_SEMANTIC ) {
            switch( m_uniforms[i].m_semantic ) {
            case SEMANTIC_MODELVIEW_PROJECTION_MATRIX:
                data = state.modelviewProjection();
                type = UNIFORM_FLOAT4X4;
                break;
            case SEMANTIC_NORMAL_MATRIX:
                data = state.normalMatrix();
                type = UNIFORM_FLOAT3X3;
                break;
            }
        }
        switch( type ) {
        case UNIFORM_SEMANTIC:
            break;
        case UNIFORM_INT:
            glUniform1iv( m_uniforms[i].m_location, 1, reinterpret_cast<const GLint*>( data ) );
            break;
        case UNIFORM_FLOAT:
            glUniform1fv( m_uniforms[i].m_location, 1, reinterpret_cast<const GLfloat*>( data ) );
            break;
        case UNIFORM_FLOAT2:
            glUniform2fv( m_uniforms[i].m_location, 1, reinterpret_cast<const GLfloat*>( data ) );
            break;
        case UNIFORM_FLOAT3:
            glUniform3fv( m_uniforms[i].m_location, 1, reinterpret_cast<const GLfloat*>( data ) );
            break;
        case UNIFORM_FLOAT4:
            glUniform4fv( m_uniforms[i].m_location, 1, reinterpret_cast<const GLfloat*>( data ) );
            break;
        case UNIFORM_FLOAT3X3:
            glUniformMatrix3fv( m_uniforms[i].m_location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>( data ) );
            break;
        case UNIFORM_FLOAT4X4:
            glUniformMatrix4fv( m_uniforms[i].m_location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>( data ) );
            break;
        }
    }
#ifdef DEBUG
    Logger log = getLogger( package + ".invoke" );
    CHECK_GL;
#endif
}


} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
