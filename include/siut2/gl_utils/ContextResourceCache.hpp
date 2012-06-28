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

/* -*- mode: C++; tab-width:4; c-basic-offset: 4; indent-tabs-mode:nil -*- */
/************************************************************************
 *
 *  File: ContextResourceCache.hpp
 *
 *  Created: 2009-08-14
 *
 *  Version:
 *
 *  Authors: Christopher Dyken <christopher.dyken@sintef.no>
 *
 *  This file is part of the siut library.
 *  Copyright (C) 2009 by SINTEF.  All rights reserved.
 *
 *  SINTEF, Pb 124 Blindern, N-0314 Oslo, Norway
 *  http://www.sintef.no
 *********************************************************************/
#pragma once


#include <GL/glew.h>

#include <vector>
#include <stdexcept>

namespace siut2 {
    namespace gl_utils {

/** Base class for specific resources handled by ContexResourceCache.
  *
  * \sa ContextResourceCache
  */
class ContextResource
{
public:
    ContextResource() {}
    virtual ~ContextResource() {};
};


/** Per-context singleton that caches GL-specific resources.
  *
  * This class represents a singleton within a OpenGL context (or a set of
  * sharing contexts) that is used to cache GL-specific resources (like static
  * textures, VBOs, and shader programs) for the various utility functions in
  * the gl_utils subtree.
  *
  * Ideally, this would be done with a key-value-map, which would make the
  * cache completely agnostic to the specific resources cached. However, since
  * the cache is queried quite often, the cache has a fixed number of slots
  * (see the ResourceSlot enum), where each slot is filled by a specific
  * utility function.
  *
  * A cache slot is fetched using the getResource function, and a cache slot
  * is set using the setResource function. The corresponding cache slot is
  * typically set the first time the utility function is called. It is an
  * error to set a cache slot multiple times. The destructor deletes all the
  * objects stored in the cache, i.e., the destructor of the individual
  * ContextResource-derived objects are called (and should do corresponding
  * cleanup of the OpenGL state).
  */
class ContextResourceCache
{
    friend class ContextResource;
public:

    /** Enumerates the slots handled by the cache.
      *
      * New utility functions should register by adding an appropriate entry
      * in this enum before RESOURCE_SLOTS_N.
      */
    enum ResourceSlot {
        RESOURCE_RENDERSTRING = 0,
        RESOURCE_RENDERCOORDAXIS,
        RESOURCE_RENDERCOORDSYS,
        RESOURCE_SLOTS_N ///< The total number of slots handled by the cache.
    };

    /** Constructor. */
    ContextResourceCache()
    {
        resource_slots_.resize( RESOURCE_SLOTS_N );
    }

    /** Destructor, deletes all managed ContextResource-derived objects. */
    ~ContextResourceCache()
    {
        for(size_t i=0; i<resource_slots_.size(); i++) {
            if( resource_slots_[i] != NULL ) {
                delete resource_slots_[i];
            }
        }
    }

    /** Fetches a resource from the cache.
      *
      * \param   slot  The slot from which to fetch the resource.
      * \returns       A pointer to the resource or NULL if resource is not set.
      * \throws        std::out_of_range if slot is illegal.
      */
    ContextResource*
    getResource( const ResourceSlot slot ) const
    {
        if( slot >= RESOURCE_SLOTS_N ) {
            throw std::out_of_range( "Illegal slot" );
        }
        else {
            return resource_slots_[ slot ];
        }
    }

    /** Sets the resource for a particular slot in the cache.
      *
      * \param   slot     The slot to set.
      * \param   resource The resource to fill the slot with.
      * \throws           std::out_of_range if slot is illegal.
      * \throws           std::runtime_error if slot is set with a NULL pointer.
      * \throws           std::runtime_error if slot is already set.
      */
    void
    setResource( const ResourceSlot slot, ContextResource* resource )
    {
        if( slot >= RESOURCE_SLOTS_N ) {
            throw std::out_of_range( "Illegal slot" );
        }
        else if( resource == NULL ) {
            throw std::runtime_error( "setting NULL resource" );
        }
        else if( resource_slots_[ slot ] != NULL ) {
            throw std::runtime_error( "multiple initializations of slot" );
        }
        else {
            resource_slots_[ slot ] = resource;
        }
    }


protected:
    std::vector<ContextResource*> resource_slots_;

    /** Copy constructor, protected to avoid copying. */
    ContextResourceCache( const ContextResourceCache& )
    {}

};



    } // of gl_utils
} // of siut

