#pragma once
#include <algorithm>
#include <librenderlist/RenderList.hpp>
#include <librenderlist/Action.hpp>

namespace librenderlist {

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




} // of namespace librenderlist
