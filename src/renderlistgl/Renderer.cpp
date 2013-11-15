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

#include <string>
#include <typeinfo>
#include <iostream>
#include <GL/glew.h>
#include <tinia/renderlist/Logger.hpp>
#include <tinia/renderlist/DataBase.hpp>
#include <tinia/renderlist/Buffer.hpp>
#include <tinia/renderlist/Shader.hpp>
#include <tinia/renderlist/Action.hpp>
#include <tinia/renderlist/Draw.hpp>
#include <tinia/renderlist/SetPixelState.hpp>
#include <tinia/renderlist/SetRasterState.hpp>
#include <tinia/renderlist/SetFramebuffer.hpp>
#include <tinia/renderlist/SetInputs.hpp>
#include <tinia/renderlist/SetLight.hpp>
#include <tinia/renderlist/SetLocalCoordSys.hpp>
#include <tinia/renderlist/SetShader.hpp>
#include <tinia/renderlist/SetUniforms.hpp>
#include <tinia/renderlist/SetFramebufferState.hpp>
#include <tinia/renderlist/SetViewCoordSys.hpp>
#include <tinia/renderlist/gl/Renderer.hpp>
#include <tinia/renderlist/gl/RenderBuffer.hpp>
#include <tinia/renderlist/gl/RenderShader.hpp>
#include <tinia/renderlist/gl/RenderSetShader.hpp>
#include <tinia/renderlist/gl/RenderSetInputs.hpp>
#include <tinia/renderlist/gl/RenderSetUniforms.hpp>
#include <tinia/renderlist/gl/RenderDraw.hpp>
#include <tinia/renderlist/gl/RenderSetLocalCoordSys.hpp>
#include <tinia/renderlist/gl/RenderState.hpp>
#include "Utils.hpp"

namespace tinia {
namespace renderlist {
namespace gl {

static const std::string package = "renderlist.gl.Renderer";

Renderer::Renderer(const DataBase &db)
    : m_db( db ),
      m_current_revision( 0 )
{}
const RenderBuffer*
Renderer::buffer( const Id id ) const
{
	std::map<Id, RenderBuffer*>::const_iterator it = m_buffers.find( id );
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
    std::map<Id, RenderShader*>::const_iterator it = m_shaders.find( id );
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

    std::list<renderlist::Buffer*> buffers;
    std::list<renderlist::Image*>  images;
    std::list<renderlist::Shader*> shaders;
    std::list<renderlist::Action*> actions;
    std::list<renderlist::Action*> draworder;
    std::list<renderlist::Item*>   keep;
    bool new_draworder;
    bool needs_pruning;

#ifdef DEBUG
    // This is only used when DEBUG is set (see line 117, "RL_LOG_DEBUG(...)")
    Revision old_revision = m_current_revision;
#endif

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

    for( std::list<Buffer*>::iterator it=buffers.begin(); it!=buffers.end(); ++it ) {
        const Buffer* src = *it;
        RenderBuffer* dst = NULL;
        std::map<Id, RenderBuffer*>::iterator jt = m_buffers.find( src->id() );
        if( jt == m_buffers.end() ) {
            dst = new RenderBuffer( *this, src->id() );
            m_buffers[ src->id() ] = dst;
        }
        else {
            dst = jt->second;
        }
        dst->pull( src );
    }
    for( std::list<Shader*>::iterator it=shaders.begin(); it!=shaders.end(); ++it ) {
        const Shader* src = *it;
        RenderShader* dst = NULL;
        std::map<Id, RenderShader*>::iterator jt = m_shaders.find( src->id() );
        if( jt == m_shaders.end() ) {
            dst = new RenderShader( *this, src->id() );
            m_shaders[ src->id() ] = dst;
        }
        else {
            dst = jt->second;
        }
        dst->pull( src );
    }
    for( std::list<Action*>::iterator it=actions.begin(); it!=actions.end(); ++it ) {
        //std::cerr << log << "processing action " << (*it)->name() << ", type=" << typeid(**it).name() << std::endl;
        std::map<Id, RenderAction*>::iterator jt = m_actions.find( (*it)->id() );
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
        for( std::list<Action*>::iterator it=draworder.begin(); it!=draworder.end(); ++it ) {
            std::map<Id, RenderAction*>::iterator  jt = m_actions.find( (*it)->id() );
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
    for(std::list<RenderAction*>::iterator it=m_draw_order.begin(); it!=m_draw_order.end(); ++it ) {
        CHECK_GL;
        (*it)->invoke( state );
        CHECK_GL;
    }

    // Reset state back
    /**
     * \todo Review this. Do we want to do this here?
     */
    glUseProgram(0);
}



} // of namespace gl
} // of namespace renderlist
} // of namespace tinia
