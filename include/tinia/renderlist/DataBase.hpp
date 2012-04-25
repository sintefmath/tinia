#pragma once
#include <string>
#include <list>
#include <unordered_map>
#include "RenderList.hpp"

namespace tinia {
namespace renderlist {

class DataBase
{
public:
    DataBase();

    /** Fetch an item (shader,buffer,image,action) by name.
      *
      * \returns The item or null if no item is found.
      */
    Item*
    itemByName( const std::string& name );

    /** Fetch an item by name, casted to a particular type.
      *
      * \returns The item or null if either the item is not found or the item
      *          is not of that type.
      */
    template<typename T>
    T
    castedItemByName( const std::string& name );

    /** Create a new buffer (vertex or index data).
      *
      * \param[in] name  Optional name that can be used for lookup.
      */
    Buffer*
    createBuffer( const std::string& name = "" );

    /** Delete a buffer. */
    void
    deleteBuffer( const Id id );

    /** Create a new shader program. */
    Shader*
    createShader( const std::string& name = "" );

    void
    deleteShader( const Id id );

    template<typename T>
    T*
    createAction( const std::string& name = "" );

    void
    deleteAction( const Id id );

    Revision
    latest( ) const;

    Revision
    bump( );

    /** Increase revision number of an item.
      *
      * \param item  The item to taint.
      * \param rethink_draworder  True if the reason for the taint is a change
      *                           that requires the draw order to be
      *                           reconsidered, or false if the change is
      *                           trivial (change of uniform value but not type).
      */
    void
    taint( Item* item, bool rethink_draworder );

    DataBase*
    drawOrderClear();

    DataBase*
    drawOrderAdd( const Id action_id );

    DataBase*
    drawOrderAdd( const std::string& action_name );

    /** Process updates done to the database prior to exporting changes.
      *
      * - Taints according to dependencies.
      * - Insert semantic update actions.
      * - Checks validity of the draw order.
      *
      * \param[in] delete_unused Deletes items that are not currently used in
      *                          the draw order.
      * \returns true If everything is ok, false if any errors were encountered.
      */
    bool
    process( bool delete_unused=false );

    /** Pull a set of changes from the database.
      *
      * \param modified_buffers Pointers to buffer that has changed since
      *                         client's revision.
      * \param modified_images  Pointers to images that has changed since
      *                         client's revision.
      * \param modified_shaders Pointers to shaders that has changed since
      *                         client's revision.
      * \param modified_actions Pointers to actions that has changed since
      *                         client's revision.
      * \param new_draworder    True if draworder has changed.
      * \param draworder        List of actions that is the current draw order
      *                         if changed since client's revision, otherwise
      *                         empty.
      * \param needs_pruning    Removal of items has occured since client's
      *                         revision.
      * \param keepers          List of items that has not been modified, but
      *                         should be keept, i.e., a client should remove
      *                         all items that is neither in a modified list nor
      *                         in keepers.
      * \param has_revision     The client's revision number.
      * \returns The current revision number.
      */
    Revision
    changes( std::list<Buffer*>&  modified_buffers,
             std::list<Image*>&   modified_images,
             std::list<Shader*>&  modified_shaders,
             std::list<Action*>&  modified_actions,
             bool&                new_draworder,
             std::list<Action*>&  draworder,
             bool&                needs_pruning,
             std::list<Item*>&    keepers,
             const Revision       has_revision ) const;

protected:
    Id                                      m_next_id;
    Revision                                m_current_rev;      ///< Current revision of the database.
    Revision                                m_deletion_rev;     ///< Revision when most recent delete was done.
    Revision                                m_draworder_rev;    ///< Revision of the draw order.
    Revision                                m_process_rev;      ///< Revision when most recent process was done.
    std::unordered_map<std::string,Item*>   m_name_map;
    std::unordered_map<Id,Buffer*>          m_buffers;
    std::unordered_map<Id,Shader*>          m_shaders;
    std::unordered_map<Id,Action*>          m_actions;
    std::list<Action*>                      m_draworder;

    Id
    newId();

    void
    attachName( const std::string& name, Item* item );

    void
    detachName( const std::string& name, Item* item );

    void
    itemDeleted();

};


} // of namespace renderlist
} // of namespace tinia

