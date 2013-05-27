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
#include "RenderList.hpp"
#include "Item.hpp"

namespace tinia {
namespace renderlist {

class Buffer : public Item
{
    friend class DataBase;
public:

    /** Get element type. */
    const ElementType
    type() const;

    /** Get element count. */
    const size_t
    count() const;

    /** Get pointer for float-based types. */
    const float*
    floatData() const;

    /** Get pointer for int-based types. */
    const int*
    intData() const;


    Buffer*
    set( const float* data, size_t count );


    Buffer*
    set( const int* data, size_t count );

protected:
    ElementType                 m_type;
    std::vector<unsigned char>  m_payload;

    Buffer( Id id, DataBase& db, const std::string& name );


};


} // of namespace renderlist
} // of namespace tinia

