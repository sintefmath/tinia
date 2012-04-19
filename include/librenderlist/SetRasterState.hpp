#pragma once
#include <algorithm>
#include <librenderlist/RenderList.hpp>
#include <librenderlist/Action.hpp>

namespace librenderlist {




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




} // of namespace librenderlist
