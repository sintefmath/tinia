#pragma once
#include <algorithm>
#include <tinia/librenderlist/DataBase.hpp>
#include <tinia/librenderlist/RenderList.hpp>
#include <tinia/librenderlist/Action.hpp>

namespace librenderlist {

class Draw : public Action
{
    friend class DataBase;
public:

    const PrimitiveType
    primitiveType() const { return m_primitive_type; }

    const bool
    isIndexed() const { return m_index_buffer_id != ~0u; }

    const Id
    indexBufferId() const { return m_index_buffer_id; }

    const size_t
    first() const { return m_first; }

    const size_t
    count() const { return m_count; }

    Draw*
    setNonIndexed( const PrimitiveType type, const size_t first, const size_t count )
    { m_primitive_type = type; m_index_buffer_id = ~0u; m_first = first; m_count = count; m_db.taint( this, true ); return this; }

    Draw*
    setIndexed( const PrimitiveType type, const Id indices, const size_t first, const size_t count )
    { m_primitive_type = type; m_index_buffer_id = indices; m_first = first; m_count = count; m_db.taint( this, true ); return this; }


private:
    PrimitiveType   m_primitive_type;
    Id              m_index_buffer_id;
    size_t          m_first;
    size_t          m_count;

    Draw( Id id, DataBase& db, const std::string& name )
        : Action( id, db, name ),
          m_primitive_type( PRIMITIVE_POINTS ),
          m_index_buffer_id( ~0u ),
          m_first( 0u ),
          m_count( 0u )
    {
    }
};

} // of namespace librenderlist
