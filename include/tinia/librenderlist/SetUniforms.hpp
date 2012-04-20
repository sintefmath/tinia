#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <tinia/librenderlist/RenderList.hpp>
#include <tinia/librenderlist/DataBase.hpp>
#include <tinia/librenderlist/Action.hpp>
#include <tinia/librenderlist/Shader.hpp>
#include <tinia/librenderlist/Logger.hpp>

namespace tinia {
namespace librenderlist {

/** Action that sets one or more uniform values of a shader program.
  *
  *
  *
  * The special 'semantic' uniform type pulls values directly from the rendering
  * state (transform matrices, current light properties etc.), and has an
  * implicit type.
  *
  */
class SetUniforms : public Action
{
    friend class DataBase;
public:

    /** Return the shader program of which uniforms should be set. */
    const Id
    shaderId() const;

    /** Return the number of uniforms set by this action. */
    const size_t
    count() const;

    /** Return the symbol of a specific uniform. */
    const std::string&
    symbol( size_t index ) const;

    /** Return the type of a specific uniform. */
    const UniformType
    type( size_t index ) const;

    /** Return the semantic of a specific uniform, if type is semantic. */
    const UniformSemantic
    semantic( size_t index ) const;

    /** Return a pointer to integer data for a specific uniform, if type is int-based. */
    const int*
    intData( size_t index ) const;

    /** Return a pointer to float data for a specific uniform, if type is float-based. */
    const float*
    floatData( size_t index ) const;

    /** Set shader of which uniforms should be set using shader id. */
    SetUniforms*
    setShader( Id shader );

    /** Set shader of which uniforms should be set using shader name. */
    SetUniforms*
    setShader( const std::string& shader_name );

    /** Clear all uniforms set by this action. */
    SetUniforms*
    clear();

    /** Set a uniform to runtime semantic binding. */
    SetUniforms*
    setSemantic( const std::string& symbol, UniformSemantic semantic );

    /** Set a uniform to a constant value. */
    SetUniforms*
    setInt1( const std::string& symbol, const int v );

    /** Set a uniform to a constant value. */
    SetUniforms*
    setFloat1( const std::string& symbol, const float v );

    /** Set a uniform to a constant value. */
    SetUniforms*
    setFloat1v( const std::string& symbol, const float *v );

    /** Set a uniform to a constant value. */
    SetUniforms*
    setFloat2v( const std::string& symbol, const float *v );

    /** Set a uniform to a constant value. */
    SetUniforms*
    setFloat3v( const std::string& symbol, const float *v );

    /** Set a uniform to a constant value. */
    SetUniforms*
    setFloat3( const std::string& symbol, const float e0, const float e1, const float e2 );

    /** Set a uniform to a constant value. */
    SetUniforms*
    setFloat4v( const std::string& symbol, const float *v );

    /** Set a uniform to a constant value. */
    SetUniforms*
    setFloat3x3v( const std::string& symbol, const float *v );

    /** Set a uniform to a constant value. */
    SetUniforms*
    setFloat4x4v( const std::string& symbol, const float *v );

protected:
    struct Uniform {
        std::string         m_symbol;
        UniformType         m_type;
        union {
            UniformSemantic m_semantic;
            int             m_int[16];
            float           m_float[16];
        }                   m_payload;
    };

    Id                      m_shader_id;       ///< Shader program to manipulate.
    std::vector<Uniform>    m_uniforms;

    SetUniforms( Id id, DataBase& db, const std::string& name )
        : Action( id, db, name ),
          m_shader_id( ~0u )
    {
    }

    size_t
    findOrAddUniform( const std::string symbol );

};

} // of namespace librenderlist
} // of namespace tinia

