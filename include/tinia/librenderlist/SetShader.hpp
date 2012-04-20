#pragma once
#include <algorithm>
#include <tinia/librenderlist/DataBase.hpp>
#include <tinia/librenderlist/RenderList.hpp>
#include <tinia/librenderlist/Action.hpp>
#include <tinia/librenderlist/Shader.hpp>

namespace tinia {
namespace librenderlist {

class SetShader : public Action
{
    friend class DataBase;
public:

    const Id
    shaderId() const { return m_shader; }

    SetShader*
    setShader( Id shader )
    { m_shader = shader; m_db.taint( this, true ); return this; }

    SetShader*
    setShader( const std::string& name )
    { Shader* s = m_db.castedItemByName<Shader*>( name );
      if( s != NULL ) {
          return setShader( s->id() );
      }
      else {
          return this;
      }
    }


private:
    Id              m_shader;

    SetShader( Id id, DataBase& db, const std::string& name )
        : Action( id, db, name ),
          m_shader( ~0u )
    {
    }
};




} // of namespace librenderlist
} // of namespace tinia

