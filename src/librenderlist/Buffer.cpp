#include <algorithm>
#include <tinia/librenderlist/DataBase.hpp>
#include <tinia/librenderlist/Buffer.hpp>


namespace librenderlist {

Buffer::Buffer( Id id, DataBase &db, const std::string& name  )
    : Item( id, db, name ),
      m_type( ELEMENT_FLOAT )
{
}

const ElementType
Buffer::type() const
{
    return m_type;
}

const size_t
Buffer::count() const
{
    switch( m_type ) {
    case ELEMENT_INT:
        return m_payload.size()/sizeof(float);
        break;
    case ELEMENT_FLOAT:
        return m_payload.size()/sizeof(int);
        break;
    }
}

const float*
Buffer::floatData() const
{
    if( m_type == ELEMENT_FLOAT ) {
        return reinterpret_cast<const float*>( m_payload.data() );
    }
    else {
        return NULL;
    }
}

const int*
Buffer::intData() const
{
    if( m_type == ELEMENT_INT ) {
        return reinterpret_cast<const int*>( m_payload.data() );
    }
    else {
        return NULL;
    }
}

Buffer*
Buffer::set( std::initializer_list<float> list )
{
    m_type = ELEMENT_FLOAT;
    m_payload.resize( sizeof(float)*list.size() );
    std::copy( list.begin(), list.end(), reinterpret_cast<float*>( m_payload.data() ) );
    m_db.taint( this, true );
    return this;
}

Buffer*
Buffer::set( std::initializer_list<int> list )
{
    m_type = ELEMENT_INT;
    m_payload.resize( sizeof(int)*list.size() );
    std::copy( list.begin(), list.end(), reinterpret_cast<int*>( m_payload.data() ) );
    m_db.taint( this, true );
    return this;
}

Buffer*
Buffer::set( const float* data, size_t count )
{
    m_type = ELEMENT_FLOAT;
    m_payload.resize( sizeof(float)*count );
    std::copy( data, data+count, reinterpret_cast<float*>( m_payload.data() ) );
    m_db.taint( this, true );
    return this;
}

Buffer*
Buffer::set( const int* data, size_t count )
{
    m_type = ELEMENT_INT;
    m_payload.resize( sizeof(int)*count );
    std::copy( data, data+count, reinterpret_cast<int*>( m_payload.data() ) );
    m_db.taint( this, true );
    return this;
}


} // of namespace librenderlist
