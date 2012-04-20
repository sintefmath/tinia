#pragma once
#include <algorithm>
#include <tinia/librenderlist/RenderList.hpp>
#include <tinia/librenderlist/Action.hpp>

namespace tinia {
namespace librenderlist {

class SetPixelState : public Action
{
    friend class DataBase;
public:
    const bool isDepthTestEnabled() const { return m_depth_test; }

    const DepthFunc depthFunc() const { return m_depth_func; }

    const bool isBlendingEnabled() const { return m_blending; }

    const BlendFunc blendSrcRGB() const { return m_blend_src_rgb; }

    const BlendFunc blendDstRGB() const { return m_blend_dst_rgb; }

    const BlendFunc blendSrcAlpha() const { return m_blend_src_alpha; }

    const BlendFunc blendDstAlpha() const { return m_blend_dst_alpha; }

    SetPixelState*
    enableDepthTest( DepthFunc depth_func = DEPTH_FUNC_LESS )
    { m_depth_test = true; m_depth_func = depth_func; m_db.taint( this, true ); return this; }

    SetPixelState*
    disableDepthTest( )
    { m_depth_test = false; m_db.taint( this, true ); return this; }

    SetPixelState*
    enableBlending( BlendFunc src_rgb, BlendFunc dst_rgb, BlendFunc src_a, BlendFunc dst_a )
    {
        m_blending = true;
        m_blend_src_rgb = src_rgb;
        m_blend_dst_rgb = dst_rgb;
        m_blend_src_alpha = src_a;
        m_blend_dst_alpha = dst_a;
        m_db.taint( this, true );
        return this;
    }

    SetPixelState*
    disableBlending( )
    { m_blending = false;  m_db.taint( this, true ); return this; }

private:
    bool            m_depth_test;
    DepthFunc       m_depth_func;
    bool            m_blending;
    BlendFunc       m_blend_src_rgb;
    BlendFunc       m_blend_dst_rgb;
    BlendFunc       m_blend_src_alpha;
    BlendFunc       m_blend_dst_alpha;


    SetPixelState( Id id, DataBase& db, const std::string& name )
        : Action( id, db, name ),
          m_depth_test( false ),
          m_depth_func( DEPTH_FUNC_LESS ),
          m_blending( false ),
          m_blend_src_rgb( BLEND_FUNC_ONE ),
          m_blend_dst_rgb( BLEND_FUNC_ZERO ),
          m_blend_src_alpha( BLEND_FUNC_ONE ),
          m_blend_dst_alpha( BLEND_FUNC_ZERO )
    {
    }
};




} // of namespace librenderlist
} // of namespace tinia

