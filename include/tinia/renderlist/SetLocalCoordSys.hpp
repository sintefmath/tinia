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
#include <tinia/renderlist/RenderList.hpp>
#include <tinia/renderlist/Action.hpp>

namespace tinia {
namespace renderlist {

class SetLocalCoordSys : public Action
{
    friend class DataBase;
public:

    const float*
    fromWorld() const { return m_from_world; }

    const float*
    toWorld() const { return m_to_world; }

    SetLocalCoordSys*
    setOrientation( const float* from_world, const float* to_world )
    {
        std::copy_n( from_world, 16, m_from_world );
        std::copy_n( to_world, 16, m_to_world );
        m_db.taint( this, false );
        return this;
    }

private:
    float           m_from_world[16];
    float           m_to_world[16];

    SetLocalCoordSys( Id id, DataBase& db, const std::string& name )
        : Action( id, db, name )
    {
        static const float unit[16] = { 1.f, 0.f, 0.f, 0.f,
                                        0.f, 1.f, 0.f, 0.f,
                                        0.f, 0.f, 1.f, 0.f,
                                        0.f, 0.f, 0.f, 1.f };
        std::copy_n( unit, 16, m_from_world );
        std::copy_n( unit, 16, m_to_world );
    }
};




} // of namespace renderlist
} // of namespace tinia

