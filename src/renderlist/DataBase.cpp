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

#include <typeinfo>
#include <iostream>
#include <tinia/renderlist/Logger.hpp>
#include <tinia/renderlist/DataBase.hpp>
#include <tinia/renderlist/Item.hpp>
#include <tinia/renderlist/Buffer.hpp>
#include <tinia/renderlist/Image.hpp>
#include <tinia/renderlist/Shader.hpp>
#include <tinia/renderlist/SetInputs.hpp>
#include <tinia/renderlist/SetViewCoordSys.hpp>
#include <tinia/renderlist/SetFramebufferState.hpp>
#include <tinia/renderlist/SetFramebuffer.hpp>
#include <tinia/renderlist/SetLocalCoordSys.hpp>
#include <tinia/renderlist/SetViewCoordSys.hpp>
#include <tinia/renderlist/SetPixelState.hpp>
#include <tinia/renderlist/SetShader.hpp>
#include <tinia/renderlist/SetUniforms.hpp>
#include <tinia/renderlist/SetRasterState.hpp>
#include <tinia/renderlist/Draw.hpp>
#include <tinia/renderlist/SetLight.hpp>


namespace tinia {
namespace renderlist {

static const std::string package = "renderlist.DataBase";

DataBase::DataBase()
    : m_next_id( 0u ),
      m_current_rev( 0u ),
      m_deletion_rev( m_current_rev ),
      m_draworder_rev( m_current_rev ),
      m_process_rev( m_current_rev )
{
}

Id
DataBase::newId()
{
    return m_next_id++;
}

Item*
DataBase::itemByName( const std::string& name )
{
    Logger log = getLogger( package + ".itemByName" );

	std::map<std::string,Item*>::iterator it = m_name_map.find( name );
    if( it != m_name_map.end() ) {
        return it->second;
    }
    else {
        //RL_LOG_WARN( log, "unable to find item with name " << name );
    }
    return NULL;
}

template<typename T>
T
DataBase::castedItemByName( const std::string& name )
{
    Logger log = getLogger( package + ".castedItemByName" );
    Item* i = itemByName( name );
    if( i != NULL ) {
        T o = dynamic_cast<T>( i );
        if( o != NULL ) {
            return o;
        }
        RL_LOG_ERROR( log, "unable to cast " << typeid(i).name() <<" to " << typeid(T).name() );
    }
    return NULL;
}
template Action*                DataBase::castedItemByName<Action*>( const std::string& );
template Buffer*                DataBase::castedItemByName<Buffer*>( const std::string& );
template Image*                 DataBase::castedItemByName<Image*>( const std::string& );
template Draw*                  DataBase::castedItemByName<Draw*>( const std::string& );
template SetFramebufferState*   DataBase::castedItemByName<SetFramebufferState*>( const std::string& );
template SetFramebuffer*        DataBase::castedItemByName<SetFramebuffer*>( const std::string& );
template SetInputs*             DataBase::castedItemByName<SetInputs*>( const std::string& );
template SetLight*              DataBase::castedItemByName<SetLight*>( const std::string& );
template SetLocalCoordSys*      DataBase::castedItemByName<SetLocalCoordSys*>( const std::string& );
template SetPixelState*         DataBase::castedItemByName<SetPixelState*>( const std::string& );
template SetShader*             DataBase::castedItemByName<SetShader*>( const std::string& );
template SetUniforms*            DataBase::castedItemByName<SetUniforms*>( const std::string& );
template SetViewCoordSys*       DataBase::castedItemByName<SetViewCoordSys*>( const std::string& );
template Shader*                DataBase::castedItemByName<Shader*>( const std::string& );
template SetRasterState*        DataBase::castedItemByName<SetRasterState*>( const std::string& );

void
DataBase::attachName( const std::string& name, Item* item )
{
    Logger log = getLogger( package + ".attachName" );
	std::map<std::string, Item*>::iterator it = m_name_map.find( name );
    if( it != m_name_map.end() ) {
        RL_LOG_ERROR( log, "name '" << name << "' already in use." );
        return;
    }
    m_name_map[ name ] = item;
}

void
DataBase::detachName( const std::string& name, Item* item )
{
    Logger log = getLogger(  package + ".detachName" );

	std::map<std::string, Item*>::iterator it = m_name_map.find( name );
    if( it == m_name_map.end() ) {
        RL_LOG_ERROR( log, "name '" << name << "' not in use" );
    }
    else if( it->second != item ) {
        RL_LOG_ERROR( log, "detaching id " << item->id() <<
                      ", but name '" << name <<
                      "' is attached to id " << it->second->id() );
    }
    else {
        m_name_map.erase( it );
    }
}

DataBase*
DataBase::drawOrderClear()
{
    m_draworder.clear();
    m_draworder_rev = ++m_current_rev;
    return this;
}

DataBase*
DataBase::drawOrderAdd( const Id action_id )
{
    Logger log = getLogger( package + ".drawOrderAdd[Id]" );

	std::map<Id,Action*>::iterator it = m_actions.find( action_id );
    if( it == m_actions.end() ) {
        RL_LOG_ERROR( log, "couldn't find any actions with id=" << action_id );
        return this;
    }
    m_draworder.push_back( it->second );
    m_draworder_rev = ++m_current_rev;
    return this;
}

DataBase*
DataBase::drawOrderAdd( const std::string& action_name )
{
    Logger log = getLogger( package + ".drawOrderAdd[string]" );

	std::map<std::string,Item*>::iterator it = m_name_map.find( action_name );
    if( it == m_name_map.end() ) {
        RL_LOG_ERROR( log, "couldn't find any items with name=" << action_name );
        return this;
    }
    Action* a = dynamic_cast<Action*>( it->second );
    if( a == NULL ) {
        RL_LOG_ERROR( log, "item with name='" << action_name <<
                      "' is not an action but " << typeid(it->second).name() );
        return this;
    }
    m_draworder.push_back( a );
    m_draworder_rev = ++m_current_rev;
    return this;
}



Revision
DataBase::changes( std::list<Buffer*>&  modified_buffers,
                   std::list<Image*>&   modified_images,
                   std::list<Shader*>&  modified_shaders,
                   std::list<Action*>&  modified_actions,
                   bool&                new_draworder,
                   std::list<Action*>&  draworder,
                   bool&                needs_pruning,
                   std::list<Item*>&    keepers,
                   const Revision       has_revision ) const
{
    Logger log = getLogger( package + ".changes" );

    if( m_process_rev < m_current_rev ) {
        RL_LOG_ERROR( log, "Changes has been done to db since last process invocation" );
    }

    keepers.clear();
    needs_pruning = has_revision < m_deletion_rev;

    // --- buffers
    modified_buffers.clear();
	for(std::map<Id,Buffer*>::const_iterator  it=m_buffers.begin(); it != m_buffers.end(); ++it ) {
        if( has_revision < it->second->m_revision ) {
            modified_buffers.push_back( it->second );
        }
        else if( needs_pruning ) {
            keepers.push_back( it->second );
        }
    }

    // --- images
    modified_images.clear();

    // --- shaders
    modified_shaders.clear();
	for(std::map<Id, Shader*>::const_iterator it=m_shaders.begin(); it!=m_shaders.end(); ++it ) {
        if( has_revision < it->second->m_revision ) {
            modified_shaders.push_back( it->second );
        }
        else if( needs_pruning ) {
            keepers.push_back( it->second );
        }
    }

    // --- actions
	for(std::map<Id, Action*>::const_iterator it=m_actions.begin(); it!=m_actions.end(); ++it ) {
        if( has_revision < it->second->m_revision ) {
            modified_actions.push_back( it->second );
        }
        else if( needs_pruning ) {
            keepers.push_back( it->second );
        }
    }

    // --- draworder
    draworder.clear();
    new_draworder = has_revision < m_draworder_rev;
    if( new_draworder ) {
        std::copy( m_draworder.begin(), m_draworder.end(), std::back_inserter(draworder) );
    }

    return m_current_rev;
}

bool
DataBase::process( bool delete_unused )
{
    Logger log = getLogger( package + ".process" );
    bool retval = true;

    std::map<Id,bool> in_use;
    std::list<Action*> draw_order;

    Id curr_shader = ~0u;
	for(std::list<Action*>::iterator it=m_draworder.begin(); it!=m_draworder.end(); ++it ) {

        // --- process SetShader action ----------------------------------------
        if( typeid(**it) == typeid(SetShader) ) {
            SetShader* a = static_cast<SetShader*>( *it );
            if( a->shaderId() == ~0u ) {
                RL_LOG_ERROR( log, "SetShader with undefined shader." );
                retval = false;
            }
            else {
				std::map<Id, Shader*>::iterator jt = m_shaders.find( a->shaderId() );
                if( jt == m_shaders.end() ) {
                    RL_LOG_ERROR( log, "SetShader with illegal shader id: " << a->shaderId() );
                    retval = false;
                }
                else {
                    Shader* s = jt->second;
                    if( a->m_revision < s->m_revision ) {
                        RL_LOG_DEBUG( log, "Shader (id=" << s->id() <<
                                      ", rev=" << s->m_revision <<
                                      ") is more recent than SetShader (id=" << a->id() <<
                                      ", rev=" << a->m_revision <<
                                      "), (taint)ing" );
                        a->m_revision = s->m_revision;
                    }
                    curr_shader = s->id();
                    if( delete_unused ) {
                        in_use[ s->id() ] = true;
                        in_use[ a->id() ] = true;
                    }
                    draw_order.push_back( a );
                }
            }
        }
        // --- process SetInputs action ----------------------------------------
        else if( typeid(**it) == typeid(SetUniforms) ) {
            SetUniforms* a = static_cast<SetUniforms*>( *it );
            if( a->shaderId() == ~0u ) {
                RL_LOG_ERROR( log, "SetUniforms with undefined shader." );
                retval = false;
            }
            else if( a->shaderId() != curr_shader ) {
                RL_LOG_ERROR( log, "SetUniforms shader id=" << a->shaderId() <<
                              " doesn't match current shader id=" << curr_shader );
                retval = false;
            }
            else {
				std::map<Id, Shader*>::iterator jt = m_shaders.find( a->shaderId() );
                if( jt == m_shaders.end() ) {
                    RL_LOG_ERROR( log, "SetUniforms with illegal shader id: " << a->shaderId() );
                    retval = false;
                }
                else {
                    Shader* s = jt->second;
                    if( a->m_revision < s->m_revision ) {
                        RL_LOG_DEBUG( log, "Shader (id=" << s->id() <<
                                      ", rev=" << s->m_revision <<
                                      ") is more recent than SetUniforms (id=" << a->id() <<
                                      ", rev=" << a->m_revision <<
                                      "), tainting" );
                        a->m_revision = s->m_revision;
                    }

                    // todo: tag semantics

                    if( delete_unused ) {
                        in_use[ a->id() ] = true;
                        in_use[ a->shaderId() ] = true;
                    }
                    draw_order.push_back( a );
                }
            }
        }
        // --- process SetInputs action ----------------------------------------
        else if( typeid(**it) == typeid(SetInputs) ) {
            SetInputs* a = static_cast<SetInputs*>( *it );
            if( a->shaderId() == ~0u ) {
                RL_LOG_ERROR( log, "SetInputs with undefined shader." );
                retval = false;
            }
            else if( a->shaderId() != curr_shader ) {
                RL_LOG_ERROR( log, "SetInputs shader id=" << a->shaderId() <<
                              " doesn't match current shader id=" << curr_shader );
                retval = false;
            }
            else {

                bool success = true;
                for(size_t i=0; i<a->count(); i++ ) {
                    if( a->bufferId(i) == ~0u ) {
                        RL_LOG_ERROR( log, "SetInputs with input from undefined buffer." );
                        success = false;
                    }
                    else {
						std::map<Id, Buffer*>::iterator jt = m_buffers.find( a->bufferId(i) );
                        if( jt == m_buffers.end() ) {
                            RL_LOG_ERROR( log, "SetInputs with input from non-existing buffer id=" << a->bufferId(i) );
                            success = false;
                        }
                        else {
                            Buffer* b = jt->second;
                            if( a->m_revision < b->m_revision ) {
                                RL_LOG_DEBUG( log, "Input buffer (id=" << b->id() << ", rev=" << b->m_revision <<
                                              ") is more recent than SetInputs (id=" << a->id() << ", rev=" << a->m_revision <<
                                              "), tainting" );
                                a->m_revision = b->m_revision;
                            }
                        }
                    }
                }
                if(success) {
                    if( delete_unused ) {
                        in_use[ a->id() ] = true;
                        in_use[ a->shaderId() ] = true;
                        for(size_t i=0; i<a->count(); i++ ) {
                            in_use[ a->bufferId(i) ] = true;
                        }
                    }
                    draw_order.push_back( a );
                }
                else {
                    retval = false;
                }
            }
        }
        // --- process SetInputs action ----------------------------------------
        else if( typeid(**it) == typeid(Draw) ) {
            Draw* a = static_cast<Draw*>( *it );
            if( a->isIndexed() ) {
                if( curr_shader == ~0u ) {
                    RL_LOG_ERROR( log, "Draw with no shader set." );
                    retval = false;
                }
                else if( a->indexBufferId() == ~0u ) {
                    RL_LOG_ERROR( log, "Draw with undefined index buffer." );
                    retval = false;
                }
                else {
					std::map<Id, Buffer*>::iterator jt = m_buffers.find( a->indexBufferId() );
                    if( jt == m_buffers.end() ) {
                        RL_LOG_ERROR( log, "Draw with indices from non-existing buffer id=" << a->indexBufferId() );
                        retval = false;
                    }
                    else {
                        Buffer* b = jt->second;
                        if( b->type() == ELEMENT_FLOAT ) {
                            RL_LOG_ERROR( log, "Draw with indices from buffer with float elements" );
                            retval = false;
                        }
                        else {
                            if( a->m_revision < b->m_revision ) {
                                RL_LOG_DEBUG( log, "Draw index buffer (id=" << b->id() << ", rev=" << b->m_revision <<
                                              ") is more recent than Draw (id=" << a->id() << ", rev=" << a->m_revision <<
                                              "), tainting" );
                                a->m_revision = b->m_revision;
                            }
                            if( delete_unused ) {
                                in_use[ a->id() ] = true;
                                in_use[ b->id() ] = true;
                            }
                            draw_order.push_back( *it );
                        }
                    }
                }
            }
            else {
                if( delete_unused ) {
                    in_use[ a->id() ] = true;
                }
                draw_order.push_back( a );
            }
        }
        // --- pass through other kinds of actions -----------------------------
        else {
            if( delete_unused ) {
                in_use[ (*it)->id() ] = true;
            }
            draw_order.push_back( *it );
        }
    }
    m_draworder.swap( draw_order );
    m_process_rev = m_current_rev;
    return retval;
}


void
DataBase::taint( Item* item, bool rethink_draworder )
{
    item->m_revision = ++m_current_rev;
    if( rethink_draworder ) {
        m_draworder_rev = m_current_rev;
    }
}

/*
    if( taint_dependencies ) {
        for( auto it=m_actions.begin(); it!=m_actions.end(); ++it ) {
            if( typeid(*(it->second)) == typeid(SetShader) ) {
                SetShader* a = static_cast<SetShader*>( it->second );
                if( a->shaderId() == shader->id() ) {
                    std::cerr << log << "tainting SetShader id=" << a->id() << std::endl;
                    a->m_revision = m_current_rev;
                }
            }
            else if( typeid(*(it->second)) == typeid(SetInputs) ) {
                SetInputs* a = static_cast<SetInputs*>( it->second );
                if( a->shaderId() == shader->id() ) {
                    std::cerr << log << "tainting SetInputs id=" << a->id() << std::endl;
                    a->m_revision = m_current_rev;
                }
            }
            else if( typeid(*(it->second)) == typeid(SetUniforms) ) {
                SetUniforms* a = static_cast<SetUniforms*>( it->second );
                if( a->shaderId() == shader->id() ) {
                    std::cerr << log << "tainting SetUniforms id=" << a->id() << std::endl;
                    a->m_revision = m_current_rev;
                }
            }
        }
    }

else if( typeid(*(it->second)) == typeid(SetInputs) ) {
    SetInputs* a = static_cast<SetInputs*>( it->second );
    if( a->shaderId() == shader->id() ) {
        std::cerr << log << "tainting SetInputs id=" << a->id() << std::endl;
        a->m_revision = m_current_rev;
    }
}
*/

void
DataBase::itemDeleted()
{
    m_deletion_rev = ++m_current_rev;
}

Buffer*
DataBase::createBuffer( const std::string& name )
{
    Id id = newId();
    Buffer* b = new Buffer( id, *this, name );
    if(!name.empty()) {
        attachName( name, b );
    }
    m_buffers[ id ] = b;
    taint( b, true );
    return b;
}

void
DataBase::deleteBuffer( const Id id )
{
	std::map<Id, Buffer*>::iterator it = m_buffers.find( id );
    if( it != m_buffers.end() ) {
        Buffer* b = it->second;
        if( !b->name().empty() ) {
            detachName( b->name(), b );
        }
        delete b;
        m_buffers.erase( it );
        itemDeleted();
    }
}


Shader*
DataBase::createShader( const std::string& name )
{
    Id id = newId();
    Shader* s = new Shader( id, *this, name );
    if(!name.empty()) {
        attachName( name, s );
    }
    m_shaders[ id ] = s;
    taint( s, true );
    return s;
}

void
DataBase::deleteShader( const Id id )
{
	std::map<Id, Shader*>::iterator it = m_shaders.find( id );
    if( it != m_shaders.end() ) {
        Shader* s = it->second;
        if( !s->name().empty() ) {
            detachName( s->name(), s );
        }
        delete s;
        m_shaders.erase( it );
        itemDeleted();
    }
}

template<typename T>
T*
DataBase::createAction( const std::string& name )
{
    Id id = newId();
    T* a = new T( id, *this, name );
    if(!name.empty()) {
        attachName( name, a );
    }
    m_actions[ id ] = a;
    taint( a, true );
    return a;
}
template Draw*                  DataBase::createAction<Draw>( const std::string& name );
template SetLight*              DataBase::createAction<SetLight>( const std::string& name );
template SetViewCoordSys*       DataBase::createAction<SetViewCoordSys>( const std::string& name );
template SetInputs*             DataBase::createAction<SetInputs>( const std::string& name );
template SetLocalCoordSys*      DataBase::createAction<SetLocalCoordSys>( const std::string& name );
template SetShader*             DataBase::createAction<SetShader>( const std::string& name );
template SetFramebuffer*        DataBase::createAction<SetFramebuffer>( const std::string& name );
template SetPixelState*         DataBase::createAction<SetPixelState>( const std::string& name );
template SetFramebufferState*   DataBase::createAction<SetFramebufferState>( const std::string& name );
template SetRasterState*        DataBase::createAction<SetRasterState>( const std::string& name );
template SetUniforms*            DataBase::createAction<SetUniforms>( const std::string& name );


void
DataBase::deleteAction( const Id id )
{
	std::map<Id, Action*>::iterator it = m_actions.find( id );
    if( it != m_actions.end() ) {
        Action* a = it->second;
        if(!a->name().empty()) {
            detachName( a->name(), a );
        }
        delete a;
        m_actions.erase( it );
        itemDeleted();
    }
}

Revision
DataBase::latest( ) const
{
    return m_current_rev;
}

Revision
DataBase::bump( )
{
    return m_current_rev++;
}

DataBase::~DataBase()
{
    for(std::map<std::string, Item*>::iterator it = m_name_map.begin(); it != m_name_map.end(); ++it) {
        delete it->second;
    }

    std::vector<Id> shadersToDelete;
    for(std::map<Id, Shader*>::iterator it = m_shaders.begin(); it != m_shaders.end(); ++it) {
        shadersToDelete.push_back(it->first);
    }

    for(size_t i = 0; i < shadersToDelete.size(); ++i) {
        deleteShader(shadersToDelete[i]);
    }

    std::vector<Id> actionsToDelete;
    for(std::map<Id, Action*>::iterator it = m_actions.begin(); it != m_actions.end(); ++it) {
        actionsToDelete.push_back(it->first);
    }

    for(size_t i = 0; i < actionsToDelete.size(); ++i) {
        deleteAction(actionsToDelete[i]);
    }

    std::vector<Id> buffersToDelete;
    for(std::map<Id, Buffer*>::iterator it = m_buffers.begin(); it != m_buffers.end(); ++it) {
        buffersToDelete.push_back(it->first);
    }

    for(size_t i = 0; i < buffersToDelete.size(); ++i) {
        deleteBuffer(buffersToDelete[i]);
    }
}

} // of namespace renderlist
} // of namespace tinia
