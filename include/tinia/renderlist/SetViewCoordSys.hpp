#pragma once
#include <algorithm>
#include <tinia/renderlist/RenderList.hpp>
#include <tinia/renderlist/Action.hpp>

namespace tinia {
namespace renderlist {

class SetViewCoordSys : public Action
{
    friend class DataBase;
public:

    const float*
    projection() const { return m_projection; }

    const float*
    projectionInverse() const { return m_projection_inverse; }

    const float*
    fromWorld() const { return m_from_world; }

    const float*
    toWorld() const { return m_to_world; }


    SetViewCoordSys*
    setProjection( const float* m4x4, const float* m4x4_inv )
    {
        std::copy_n( m4x4, 16, m_projection );
        std::copy_n( m4x4_inv, 16, m_projection_inverse );
        m_db.taint( this, false );
        return this;
    }

    SetViewCoordSys*
    setOrientation( const float* from_world, const float* to_world )
    {
        std::copy_n( from_world, 16, m_from_world );
        std::copy_n( to_world, 16, m_to_world );
        m_db.taint( this, false );
        return this;
    }

private:
    float     m_projection[16];
    float     m_projection_inverse[16];
    float     m_from_world[16];
    float     m_to_world[16];

    SetViewCoordSys( Id id, DataBase& db, const std::string& name )
        : Action( id, db, name )
    {
        static const float unit[16] = { 1.f, 0.f, 0.f, 0.f,
                                          0.f, 1.f, 0.f, 0.f,
                                          0.f, 0.f, 1.f, 0.f,
                                          0.f, 0.f, 0.f, 1.f };
        std::copy_n( unit, 16, m_projection );
        std::copy_n( unit, 16, m_projection_inverse );
        std::copy_n( unit, 16, m_from_world );
        std::copy_n( unit, 16, m_to_world );
    }
};




} // of namespace renderlist
} // of namespace tinia

