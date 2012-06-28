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
#include <unordered_map>
#include <list>
#include <boost/utility.hpp>
#include <tinia/renderlist/RenderList.hpp>

namespace tinia {
namespace renderlist {
namespace gl {

class Renderer;
class RenderBuffer;
class RenderShader;
class RenderState;

class RenderItem
{
public:
    RenderItem( Renderer& renderer, Id id )
        : m_renderer( renderer ),
          m_id( id )
    {}

protected:
    Renderer&   m_renderer;
    Id          m_id;
};

class RenderAction : public RenderItem
{
public:
    RenderAction( Renderer& renderer, Id id )
        : RenderItem( renderer, id )
    {}


    virtual void
    invoke( RenderState& state ) = 0;

};

class Renderer : public boost::noncopyable
{
public:
    Renderer( const DataBase& db );

    /** Pulls changes from database. */
    void
    pull();

    const RenderBuffer*
    buffer( const Id id ) const;

    const RenderShader*
    shader( const Id id ) const;

    void
    render( unsigned int  fbo,
            const float*  projection,
            const float*  projection_inverse,
            const float*  modelview,
            const float*  modelview_inverse,
            const int     width,
            const int     height );


protected:
    const DataBase&                         m_db;
    Revision                                m_current_revision;
    std::unordered_map<Id,RenderBuffer*>    m_buffers;
    std::unordered_map<Id,RenderShader*>    m_shaders;
    std::unordered_map<Id,RenderAction*>    m_actions;
    std::list<RenderAction*>                m_draw_order;

};


} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
