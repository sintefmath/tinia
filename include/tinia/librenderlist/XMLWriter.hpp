#pragma once
#include <string>
#include <sstream>
#include <tinia/librenderlist/RenderList.hpp>

namespace tinia {
namespace librenderlist {



template<typename TYPE>
void
encodeArray( std::stringstream& o, const Encoding encoding, const TYPE* data, const size_t count );


std::string
getUpdateXML( const DataBase* database, const Encoding encoding, const Revision has_revision );


} // of namespace librenderlist
} // of namespace tinia

