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

#pragma once
#include <iostream>
#include <string>
#include <tinia/renderlist/RenderList.hpp>
#include <tinia/renderlist/DataBase.hpp>
#include <tinia/renderlist/Action.hpp>
#include <tinia/renderlist/Shader.hpp>
#include <tinia/renderlist/Buffer.hpp>

namespace tinia {
namespace renderlist {

/** Maps buffer data to shader inputs.
  *
  * This action equals an OpenGL VAO, where invoking it equals binding that VAO.
  * It is linked to a specific shader, since the numbering of the shader inputs
  * is dependent on the shader.
  *
  */
class SetInputs : public Action
{
    friend class DataBase;
public:

    const Id
    shaderId() const { return m_shader_id; }

    SetInputs*
    setShader( Id shader )
    { m_shader_id = shader; m_db.taint( this, true ); return this; }

    SetInputs*
    setShader( const std::string& name )
    {  Shader* s = m_db.castedItemByName<Shader*>( name );
        if( s != NULL ) { return setShader( s->id() ); } return this; }

    /** Return the number of inputs set by this action. */
    const size_t
    count() const { return m_inputs.size(); }

    const std::string&
    symbol( size_t index ) const { return m_inputs[ index ].m_symbol; }

    const Id
    bufferId( size_t index ) const { return m_inputs[ index ].m_buffer; }

    const unsigned int
    components( size_t index ) const { return m_inputs[ index ].m_components; }

    const unsigned int
    offset( size_t index ) const { return m_inputs[ index ].m_offset; }

    const unsigned int
    stride( size_t index ) const { return m_inputs[ index ].m_stride; }

    SetInputs*
    clearInputs()
    { m_inputs.clear(); m_db.taint( this, true ); return this; }

    SetInputs*
    setInput( const std::string&  symbol,
              const std::string&  buffer_name,
              const unsigned int  components,
              const unsigned int  offset = 0,
              const unsigned int  stride = 0 )
    {
        Buffer* b = m_db.castedItemByName<Buffer*>( buffer_name );
        if( b != NULL ) {
            return setInput( symbol, b->id(), components, offset, stride );
        }
        else {
            return this;
        }
    }


    SetInputs*
    setInput( const std::string&  symbol,
              const Id            buffer,
              const unsigned int  components,
              const unsigned int  offset = 0,
              const unsigned int  stride = 0 )
    {
        Input i;
        i.m_symbol = symbol;
        i.m_buffer = buffer;
        i.m_components = components;
        i.m_offset = offset;
        i.m_stride = stride == 0 ? components : stride;
        m_inputs.push_back( i );
        m_db.taint( this, true );
        return this;
    }


protected:
    Id                  m_shader_id;       ///< Shader program to manipulate.
    struct Input {
        std::string     m_symbol;       ///< Input attribute symbol in shader,
        Id              m_buffer;       ///< Id of buffer to fetch data from.
        unsigned int    m_components;   ///< Number of components in input (2D, 3D, ..).
        unsigned int    m_offset;       ///< Number of elements to offset into buffer.
        unsigned int    m_stride;       ///< Number of elements between adjacent elements, zero is tightly packed.
    };
    std::vector<Input>  m_inputs;       ///< Inputs.


    SetInputs( Id id, DataBase& db, const std::string& name )
        : Action( id, db, name ),
          m_shader_id( ~0u )
    {}


};

} // of namespace renderlist
} // of namespace tinia

