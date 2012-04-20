#include <string>
#include <typeinfo>
#include <iostream>
#include <GL/glew.h>
#include <tinia/librenderlist/Logger.hpp>
#include <tinia/librenderlist/DataBase.hpp>
#include <tinia/librenderlist/Buffer.hpp>
#include <tinia/librenderlist/Shader.hpp>
#include <tinia/librenderlist/Action.hpp>
#include <tinia/librenderlist/Draw.hpp>
#include <tinia/librenderlist/SetPixelState.hpp>
#include <tinia/librenderlist/SetRasterState.hpp>
#include <tinia/librenderlist/SetFramebuffer.hpp>
#include <tinia/librenderlist/SetInputs.hpp>
#include <tinia/librenderlist/SetLight.hpp>
#include <tinia/librenderlist/SetLocalCoordSys.hpp>
#include <tinia/librenderlist/SetShader.hpp>
#include <tinia/librenderlist/SetUniforms.hpp>
#include <tinia/librenderlist/SetFramebufferState.hpp>
#include <tinia/librenderlist/SetViewCoordSys.hpp>
#include <tinia/librenderlist/gl/Renderer.hpp>
#include <tinia/librenderlist/gl/RenderBuffer.hpp>
#include <tinia/librenderlist/gl/RenderShader.hpp>
#include <tinia/librenderlist/gl/RenderSetShader.hpp>
#include <tinia/librenderlist/gl/RenderSetInputs.hpp>
#include <tinia/librenderlist/gl/RenderSetUniforms.hpp>
#include <tinia/librenderlist/gl/RenderDraw.hpp>
#include <tinia/librenderlist/gl/RenderSetLocalCoordSys.hpp>
#include <tinia/librenderlist/gl/RenderState.hpp>
#include "Utils.hpp"

namespace librenderlist {
namespace gl {

static const std::string package = "librenderlist.gl.Renderer";

Renderer::Renderer(const DataBase &db)
    : m_db( db ),
      m_current_revision( 0 )
{}
const RenderBuffer*
Renderer::buffer( const Id id ) const
{
    auto it = m_buffers.find( id );
    if( it == m_buffers.end() ) {
        return NULL;
    }
    else {
        return it->second;
    }
}

const RenderShader*
Renderer::shader( const Id id ) const
{
    auto it = m_shaders.find( id );
    if( it == m_shaders.end() ) {
        return NULL;
    }
    else {
        return it->second;
    }
}


void
Renderer::pull()
{
    Logger log = getLogger( package + ".update" );
    if( m_db.latest()== m_current_revision ) {
        // No changes.
        return;
    }

    std::list<librenderlist::Buffer*> buffers;
    std::list<librenderlist::Image*>  images;
    std::list<librenderlist::Shader*> shaders;
    std::list<librenderlist::Action*> actions;
    std::list<librenderlist::Action*> draworder;
    std::list<librenderlist::Item*>   keep;
    bool new_draworder;
    bool needs_pruning;


    Revision old_revision = m_current_revision;
    m_current_revision = m_db.changes( buffers,
                                       images,
                                       shaders,
                                       actions,
                                       new_draworder,
                                       draworder,
                                       needs_pruning,
                                       keep,
                                       m_current_revision );
    RL_LOG_DEBUG( log, "pull from " << old_revision << " to " << m_current_revision );

    for( auto it=buffers.begin(); it!=buffers.end(); ++it ) {
        const Buffer* src = *it;
        RenderBuffer* dst = NULL;
        auto jt = m_buffers.find( src->id() );
        if( jt == m_buffers.end() ) {
            dst = new RenderBuffer( *this, src->id() );
            m_buffers[ src->id() ] = dst;
        }
        else {
            dst = jt->second;
        }
        dst->pull( src );
    }
    for( auto it=shaders.begin(); it!=shaders.end(); ++it ) {
        const Shader* src = *it;
        RenderShader* dst = NULL;
        auto jt = m_shaders.find( src->id() );
        if( jt == m_shaders.end() ) {
            dst = new RenderShader( *this, src->id() );
            m_shaders[ src->id() ] = dst;
        }
        else {
            dst = jt->second;
        }
        dst->pull( src );
    }
    for( auto it=actions.begin(); it!=actions.end(); ++it ) {
        //std::cerr << log << "processing action " << (*it)->name() << ", type=" << typeid(**it).name() << std::endl;
        auto jt = m_actions.find( (*it)->id() );
        if( typeid(**it) == typeid(Draw) ) {
            const Draw* src = static_cast<const Draw*>( *it );
            RenderDraw* dst = NULL;
            if( jt == m_actions.end() ) {
                dst = new RenderDraw( *this, src->id() );
                m_actions[ src->id() ] = dst;
            }
            else {
                dst = static_cast<RenderDraw*>( jt->second );
            }
            dst->pull( src );
        }
        else if( typeid(**it) == typeid(SetPixelState) ) {
            //const SetPixelState* a = static_cast<const SetPixelState*>( *it );
        }
        else if( typeid(**it) == typeid(SetRasterState) ) {
            //const SetRasterState* a = static_cast<const SetRasterState*>( *it );
        }
        else if( typeid(**it) == typeid(SetFramebufferState) ) {
            //const SetFramebufferState* a = static_cast<const SetFramebufferState*>( *it );
        }
        else if( typeid(**it) == typeid(SetFramebuffer) ) {
            //const SetFramebuffer* a = static_cast<const SetFramebuffer*>( *it );
        }
        else if( typeid(**it) == typeid(SetInputs) ) {
            const SetInputs* src = static_cast<const SetInputs*>( *it );
            RenderSetInputs* dst = NULL;
            if( jt == m_actions.end() ) {
                dst = new RenderSetInputs( *this, src->id() );
                m_actions[ src->id() ] = dst;
            }
            else {
                dst = static_cast<RenderSetInputs*>( jt->second );
            }
            dst->pull( src );
        }
        else if( typeid(**it) == typeid(SetLight) ) {
            //const SetLight* a = static_cast<const SetLight*>( *it );
        }
        else if( typeid(**it) == typeid(SetLocalCoordSys) ) {
            const SetLocalCoordSys* src = static_cast<const SetLocalCoordSys*>( *it );
            RenderSetLocalCoordSys* dst = NULL;
            if( jt == m_actions.end() ) {
                dst = new RenderSetLocalCoordSys( *this, src->id() );
                m_actions[ src->id() ] = dst;
            }
            else {
                dst = static_cast<RenderSetLocalCoordSys*>( jt->second );
            }
            dst->pull( src );
        }
        else if( typeid(**it) == typeid(SetShader) ) {
            const SetShader* src = static_cast<const SetShader*>( *it );
            RenderSetShader* dst = NULL;
            if( jt == m_actions.end() ) {
                dst = new RenderSetShader( *this, src->id() );
                m_actions[ src->id() ] = dst;
            }
            else {
                dst = static_cast<RenderSetShader*>( jt->second );
            }
            dst->pull( src );
        }
        else if( typeid(**it) == typeid(SetUniforms) ) {
            const SetUniforms* src = static_cast<const SetUniforms*>( *it );
            RenderSetUniforms* dst = NULL;
            if( jt == m_actions.end() ) {
                dst = new RenderSetUniforms( *this, src->id() );
                m_actions[ src->id() ] = dst;
            }
            else {
                dst = static_cast<RenderSetUniforms*>( jt->second );
            }
            dst->pull( src );
        }
        else if( typeid(**it) == typeid(SetViewCoordSys) ) {
            //SetViewCoordSys* a = static_cast<SetViewCoordSys*>( *it );
        }
        else {
            RL_LOG_ERROR( log, "unsupported action " << (*it)->name() <<
                          ", type=" << typeid(**it).name() );
        }
    }

    if( new_draworder ) {
        m_draw_order.clear();
        for( auto it=draworder.begin(); it!=draworder.end(); ++it ) {
            auto jt = m_actions.find( (*it)->id() );
            if( jt != m_actions.end() ) {
                m_draw_order.push_back( jt->second );
            }
        }
    }
    CHECK_GL;
    RL_LOG_DEBUG( log, "Updated renderlist" );
}

void
Renderer::render( unsigned int  fbo,
                  const float*  projection,
                  const float*  projection_inverse,
                  const float*  modelview,
                  const float*  modelview_inverse,
                  const int     width,
                  const int     height )
{
    Logger log = getLogger( package + ".render" );

    CHECK_GL;
    glBindFramebuffer( GL_FRAMEBUFFER, fbo );
    glViewport( 0, 0, width, height );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glEnable( GL_DEPTH_TEST );

    /*
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf( projection );
    glMatrixMode( GL_MODELVIEW );
    glLoadMatrixf( modelview );
    glUseProgram( 0 );
    glBegin( GL_TRIANGLES );
    glColor3f( 1.0, 0.0, 0.0 );
    glVertex3f( 1.0, 0.0, 0.0 );
    glVertex3f( 0.0, 0.0, 0.0 );
    glVertex3f( 0.0, 1.0, 0.0 );
    glEnd();
*/


    RenderState state;
    state.setView( projection,
                   projection_inverse,
                   modelview,
                   modelview_inverse );
    for(auto it=m_draw_order.begin(); it!=m_draw_order.end(); ++it ) {
        CHECK_GL;
        (*it)->invoke( state );
        CHECK_GL;
    }
}



} // of namespace gl
} // of namespace librenderlist
