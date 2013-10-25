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
#include <tinia/renderlist/DataBase.hpp>
#include <tinia/renderlist/Buffer.hpp>
#include <stdexcept>

namespace tinia {
namespace renderlist {

Buffer::Buffer( Id id, DataBase &db, const std::string& name  )
    : Item( id, db, name ),
      m_type( ELEMENT_FLOAT )
{
}

const ElementType
Buffer::type() const
{
    return m_type;
}

const size_t
Buffer::count() const
{
    switch( m_type ) {
    case ELEMENT_INT:
        return m_payload.size()/sizeof(float);
        break;
    case ELEMENT_FLOAT:
        return m_payload.size()/sizeof(int);
        break;
    }
    throw std::logic_error("Unknown type specificied, could not count items.");
    return 0;
}

const float*
Buffer::floatData() const
{
    if( m_type == ELEMENT_FLOAT ) {
        return reinterpret_cast<const float*>( &m_payload[0] );
    }
    else {
        return NULL;
    }
}

const int*
Buffer::intData() const
{
    if( m_type == ELEMENT_INT ) {
        return reinterpret_cast<const int*>( &m_payload[0] );
    }
    else {
        return NULL;
    }
}

Buffer*
Buffer::set( const float* data, size_t count )
{
    m_type = ELEMENT_FLOAT;
    m_payload.resize( sizeof(float)*count );
    std::copy( data, data+count, reinterpret_cast<float*>( &m_payload[0] ) );
    m_db.taint( this, true );
    return this;
}

Buffer*
Buffer::set( const int* data, size_t count )
{
    m_type = ELEMENT_INT;
    m_payload.resize( sizeof(int)*count );
	std::copy( data, data+count, reinterpret_cast<int*>( &m_payload[0] ) );
    m_db.taint( this, true );
    return this;
}


} // of namespace renderlist
} // of namespace tinia
