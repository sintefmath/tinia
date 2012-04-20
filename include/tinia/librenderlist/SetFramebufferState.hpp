#pragma once
#include <algorithm>
#include <tinia/librenderlist/RenderList.hpp>
#include <tinia/librenderlist/Action.hpp>

namespace tinia {
namespace librenderlist {

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




} // of namespace librenderlist
} // of namespace tinia

