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
#include <algorithm>
#include <tinia/renderlist/DataBase.hpp>
#include <tinia/renderlist/RenderList.hpp>
#include <tinia/renderlist/Action.hpp>
#include <tinia/renderlist/Shader.hpp>

namespace tinia {
namespace renderlist {

class SetShader : public Action
{
    friend class DataBase;
public:

    const Id
    shaderId() const { return m_shader; }

    SetShader*
    setShader( Id shader )
    { m_shader = shader; m_db.taint( this, true ); return this; }

    SetShader*
    setShader( const std::string& name )
    { Shader* s = m_db.castedItemByName<Shader*>( name );
      if( s != NULL ) {
          return setShader( s->id() );
      }
      else {
          return this;
      }
    }


private:
    Id              m_shader;

    SetShader( Id id, DataBase& db, const std::string& name )
        : Action( id, db, name ),
          m_shader( ~0u )
    {
    }
};




} // of namespace renderlist
} // of namespace tinia

