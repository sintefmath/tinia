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




} // of namespace renderlist
} // of namespace tinia

