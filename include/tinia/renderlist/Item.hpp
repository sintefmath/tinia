#pragma once
#include <boost/utility.hpp>
#include <tinia/renderlist/RenderList.hpp>

namespace tinia {
namespace renderlist {

class Item : public boost::noncopyable
{
    friend class DataBase;
public:
    const Id
    id() const
    { return m_id; }

    const std::string&
    name() const
    { return m_name; }

protected:
    Id          m_id;
    DataBase&   m_db;
    std::string m_name;
    Revision    m_revision;

    Item( Id id, DataBase& db, const std::string& name )
        : m_id( id ), m_db( db ), m_name( name )
    {}

    virtual ~Item() {}

};



} // of namespace renderlist
} // of namespace tinia

