#pragma once
#include "RenderList.hpp"
#include "Item.hpp"
#include <initializer_list>

namespace librenderlist {

class Buffer : public Item
{
    friend class DataBase;
public:

    /** Get element type. */
    const ElementType
    type() const;

    /** Get element count. */
    const size_t
    count() const;

    /** Get pointer for float-based types. */
    const float*
    floatData() const;

    /** Get pointer for int-based types. */
    const int*
    intData() const;

    Buffer*
    set( std::initializer_list<float> list );

    Buffer*
    set( const float* data, size_t count );

    Buffer*
    set( std::initializer_list<int> list );

    Buffer*
    set( const int* data, size_t count );

protected:
    ElementType                 m_type;
    std::vector<unsigned char>  m_payload;

    Buffer( Id id, DataBase& db, const std::string& name );


};


} // of namespace librenderlist
