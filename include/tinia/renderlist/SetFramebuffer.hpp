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
#include <tinia/renderlist/DataBase.hpp>
#include <tinia/renderlist/Image.hpp>

namespace tinia {
namespace renderlist {

/** Directs rendering to a framebuffer.
  *
  * This action equals an OpenGL FBO, where invoking it equals binding that FBO,
  * and the actual rendering buffer contents are managed by Image.
  *
  */
class SetFramebuffer : public Action
{
    friend class DataBase;
public:
    const Id
    imageId() const { return m_image_id; }

    /** Direct rendering to a specific image using an id. */
    SetFramebuffer*
    setImage( Id image_id )
    { m_image_id = image_id; m_db.taint( this, true ); return this; }

    /** Direct rendering to a specific image using a name. */
    SetFramebuffer*
    setImage( const std::string& image_name )
    {
        Image* i = m_db.castedItemByName<Image*>( image_name );
        if( i != NULL ) {
            m_image_id = i->id(); m_db.taint( this, true );
        }
        return this;
    }

    /** Direct rendering to the default framebuffer. */
    SetFramebuffer*
    setDefault()
    { m_image_id = ~0u; m_db.taint( this, true ); return this; }


private:
    Id              m_image_id;

    SetFramebuffer( Id id, DataBase& db, const std::string& name )
        : Action( id, db, name ),
          m_image_id( ~0u )
    {
    }
};




} // of namespace renderlist
} // of namespace tinia

