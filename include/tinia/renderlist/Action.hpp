#pragma once
#include <string>
#include "RenderList.hpp"
#include "Item.hpp"

namespace tinia {
namespace renderlist {

class Action : public Item
{
    friend class DataBase;
public:

protected:
    Action( Id id, DataBase& db, const std::string& name )
        : Item( id, db, name )
    {}

    virtual ~Action() {}

};


} // of namespace renderlist
} // of namespace tinia
