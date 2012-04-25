#pragma once
#include "RenderList.hpp"
#include "Item.hpp"

namespace tinia {
namespace renderlist {

class Image : public Item
{
    friend class DataBase;
public:


protected:

    Image( Id id, DataBase& db, const std::string& name );

};


} // of namespace renderlist
} // of namespace tinia

