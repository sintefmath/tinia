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

