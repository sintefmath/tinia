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

class SetFramebufferState : public Action
{
    friend class DataBase;
public:


    const int* colorWriteMask() const { return m_color_writemask; }

    const bool depthWriteMask() const { return m_depth_writemask; }

    SetFramebufferState*
    setColorWritemask( bool red, bool green, bool blue, bool alpha = true )
    {
        m_color_writemask[0] = red ? 1 : 0;
        m_color_writemask[1] = green ? 1 : 0;
        m_color_writemask[2] = blue ? 1 : 0;
        m_color_writemask[3] = alpha ? 1 : 0;
        m_db.taint( this, true );
        return this;
    }

    SetFramebufferState*
    setDepthWritemask( bool writemask )
    { m_depth_writemask = writemask; m_db.taint( this, true ); return this; }

private:
    int    m_color_writemask[4];
    bool   m_depth_writemask;

    SetFramebufferState( Id id, DataBase& db, const std::string& name )
        : Action( id, db, name )
    {
        std::fill_n( m_color_writemask, 4, 1 );
        m_depth_writemask = true;
    }
};




} // of namespace renderlist
} // of namespace tinia

