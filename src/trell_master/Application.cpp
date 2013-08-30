/* Copyright STIFTELSEN SINTEF 2013
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
#include <sstream>
#include "Application.hpp"

namespace tinia {
namespace trell {
namespace impl {

Application::Application( const std::string& app_bin,
                          const std::string& app_path )
    : m_app_bin( app_bin ),
      m_app_path( app_path )
{
    
}


bool
Application::refresh()
{
    return false;
}


const std::string
Application::xml() const
{
    std::stringstream xml;
    xml << "    <application binary=\"" << m_app_bin << "\"/>\n";
    return xml.str();
}

} // of namespace impl
} // of namespace trell
} // of namespace tinia
