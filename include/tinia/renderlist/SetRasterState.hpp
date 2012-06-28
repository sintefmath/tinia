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




class SetRasterState : public Action
{
    friend class DataBase;
public:

private:
    float       m_point_size;
    bool        m_cull_face;
    CullFace    m_cull_face_mode;
    PolygonMode m_polygon_mode_front;
    PolygonMode m_polygon_mode_back;
    float       m_polygon_offset_factor;
    float       m_polygon_offset_units;
    bool        m_polygon_offset_point;
    bool        m_polygon_offset_line;
    bool        m_polygon_offset_fill;

    SetRasterState( Id id, DataBase& db, const std::string& name )
    : Action( id, db, name ),
      m_cull_face( false ),
      m_cull_face_mode( CULL_BACK ),
      m_polygon_mode_front( POLYGON_MODE_FILL ),
      m_polygon_mode_back( POLYGON_MODE_FILL ),
      m_polygon_offset_factor( 0.f ),
      m_polygon_offset_units( 0.f ),
      m_polygon_offset_point( false ),
      m_polygon_offset_line( false ),
      m_polygon_offset_fill( false )
    {}
};




} // of namespace renderlist
} // of namespace tinia

