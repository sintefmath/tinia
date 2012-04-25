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

