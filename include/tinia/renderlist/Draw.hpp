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

namespace tinia {
namespace renderlist {

class Draw : public Action
{
    friend class DataBase;
public:

    const PrimitiveType
    primitiveType() const { return m_primitive_type; }

    const bool
    isIndexed() const { return m_index_buffer_id != ~0u; }

    const Id
    indexBufferId() const { return m_index_buffer_id; }

    const size_t
    first() const { return m_first; }

    const size_t
    count() const { return m_count; }

    Draw*
    setNonIndexed( const PrimitiveType type, const size_t first, const size_t count )
    { m_primitive_type = type; m_index_buffer_id = ~0u; m_first = first; m_count = count; m_db.taint( this, true ); return this; }

    Draw*
    setIndexed( const PrimitiveType type, const Id indices, const size_t first, const size_t count )
    { m_primitive_type = type; m_index_buffer_id = indices; m_first = first; m_count = count; m_db.taint( this, true ); return this; }


private:
    PrimitiveType   m_primitive_type;
    Id              m_index_buffer_id;
    size_t          m_first;
    size_t          m_count;

    Draw( Id id, DataBase& db, const std::string& name )
        : Action( id, db, name ),
          m_primitive_type( PRIMITIVE_POINTS ),
          m_index_buffer_id( ~0u ),
          m_first( 0u ),
          m_count( 0u )
    {
    }
};

} // of namespace renderlist
} // of namespace tinia

