#pragma once
#include <unordered_map>
#include <list>
#include <boost/utility.hpp>
#include <tinia/librenderlist/RenderList.hpp>

namespace tinia {
namespace librenderlist {
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
} // of namespace librenderlist
} // of namespace tinia
