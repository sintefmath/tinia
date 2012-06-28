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

#include <tinia/renderlist/SetUniforms.hpp>

namespace tinia {
namespace renderlist {

const Id
SetUniforms::shaderId() const
{
    return m_shader_id;
}

const size_t
SetUniforms::count() const
{
    return m_uniforms.size();
}

const std::string&
SetUniforms::symbol( size_t index ) const
{
    return m_uniforms[index].m_symbol;
}

const UniformType
SetUniforms::type( size_t index ) const
{
    return m_uniforms[index].m_type;
}

const UniformSemantic
SetUniforms::semantic( size_t index ) const
{
    return m_uniforms[index].m_payload.m_semantic;
}

const int*
SetUniforms::intData( size_t index ) const
{
    return m_uniforms[index].m_payload.m_int;
}

const float*
SetUniforms::floatData( size_t index ) const
{
    return m_uniforms[index].m_payload.m_float;
}

SetUniforms*
SetUniforms::setShader( Id shader )
{
    m_shader_id = shader;
    m_db.taint( this, true );
    return this;
}

SetUniforms*
SetUniforms::setShader( const std::string& shader_name )
{
    Shader* s = m_db.castedItemByName<Shader*>( shader_name );
    if( s != NULL ) {
        return setShader( s->id() );
    }
    else {
        return this;
    }
}

SetUniforms*
SetUniforms::clear()
{
    m_uniforms.clear();
    m_db.taint( this, true );
    return this;
}

size_t
SetUniforms::findOrAddUniform( const std::string symbol )
{
    for(size_t i=0; i<m_uniforms.size(); i++ ) {
        if( m_uniforms[i].m_symbol == symbol ) {
            return i;
        }
    }
    size_t ix = m_uniforms.size();
    m_uniforms.resize( ix + 1);
    m_uniforms.back().m_symbol = symbol;
    m_uniforms.back().m_type = UNIFORM_SEMANTIC;
    return ix;
}


SetUniforms*
SetUniforms::setSemantic( const std::string& symbol, UniformSemantic semantic )
{
    Uniform& u = m_uniforms[ findOrAddUniform( symbol ) ];
    u.m_type = UNIFORM_SEMANTIC;
    u.m_payload.m_semantic = semantic;
    m_db.taint( this, true );   // semantic always rethinks draworder.
    return this;
}

SetUniforms*
SetUniforms::setInt1( const std::string& symbol, const int v )
{
    Uniform& u = m_uniforms[ findOrAddUniform( symbol ) ];
    bool rethink = u.m_type != UNIFORM_INT;
    u.m_type = UNIFORM_INT;
    u.m_payload.m_int[0] = v;
    m_db.taint( this, rethink );
    return this;
}


SetUniforms*
SetUniforms::setFloat1( const std::string& symbol, const float v )
{
    Uniform& u = m_uniforms[ findOrAddUniform( symbol ) ];
    bool rethink = u.m_type != UNIFORM_FLOAT;
    u.m_type = UNIFORM_FLOAT;
    u.m_payload.m_float[0] = v;
    m_db.taint( this, rethink );
    return this;
}


SetUniforms*
SetUniforms::setFloat1v( const std::string& symbol, const float *v )
{
    Uniform& u = m_uniforms[ findOrAddUniform( symbol ) ];
    bool rethink = u.m_type != UNIFORM_FLOAT;
    u.m_type = UNIFORM_FLOAT;
    std::copy_n( v, 1, u.m_payload.m_float );
    m_db.taint( this, rethink );
    return this;
}

SetUniforms*
SetUniforms::setFloat2v( const std::string& symbol, const float *v )
{
    Uniform& u = m_uniforms[ findOrAddUniform( symbol ) ];
    bool rethink = u.m_type != UNIFORM_FLOAT2;
    u.m_type = UNIFORM_FLOAT2;
    std::copy_n( v, 2, u.m_payload.m_float );
    m_db.taint( this, rethink );
    return this;
}

SetUniforms*
SetUniforms::setFloat3v( const std::string& symbol, const float *v )
{
    Uniform& u = m_uniforms[ findOrAddUniform( symbol ) ];
    bool rethink = u.m_type != UNIFORM_FLOAT3;
    u.m_type = UNIFORM_FLOAT3;
    std::copy_n( v, 3, u.m_payload.m_float );
    m_db.taint( this, rethink );
    return this;
}

SetUniforms*
SetUniforms::setFloat3( const std::string& symbol, const float e0, const float e1, const float e2 )
{
    Uniform& u = m_uniforms[ findOrAddUniform( symbol ) ];
    bool rethink = u.m_type != UNIFORM_FLOAT3;
    u.m_type = UNIFORM_FLOAT3;
    u.m_payload.m_float[0] = e0;
    u.m_payload.m_float[1] = e1;
    u.m_payload.m_float[2] = e2;
    m_db.taint( this, rethink );
    return this;
}

SetUniforms*
SetUniforms::setFloat4v( const std::string& symbol, const float *v )
{
    Uniform& u = m_uniforms[ findOrAddUniform( symbol ) ];
    bool rethink = u.m_type != UNIFORM_FLOAT4;
    u.m_type = UNIFORM_FLOAT4;
    std::copy_n( v, 4, u.m_payload.m_float );
    m_db.taint( this, rethink );
    return this;
}

SetUniforms*
SetUniforms::setFloat3x3v( const std::string& symbol, const float *v )
{
    Uniform& u = m_uniforms[ findOrAddUniform( symbol ) ];
    bool rethink = u.m_type != UNIFORM_FLOAT3X3;
    u.m_type = UNIFORM_FLOAT3X3;
    std::copy_n( v, 9, u.m_payload.m_float );
    m_db.taint( this, rethink );
    return this;
}

SetUniforms*
SetUniforms::setFloat4x4v( const std::string& symbol, const float *v )
{
    Uniform& u = m_uniforms[ findOrAddUniform( symbol ) ];
    bool rethink = u.m_type != UNIFORM_FLOAT4X4;
    u.m_type = UNIFORM_FLOAT4X4;
    std::copy_n( v, 16, u.m_payload.m_float );
    m_db.taint( this, rethink );
    return this;
}




} // of namespace renderlist
} // of namespace tinia
