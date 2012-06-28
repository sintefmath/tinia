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
#include <string>
#include <iostream>

namespace tinia {
namespace renderlist {

typedef std::string Logger;

static inline Logger getLogger( const std::string name )
{
    return name;
}

#define RL_LOG_TRACE(a,b) do { std::cerr << "[T] " << a << ": " << b << std::endl; } while(0)
#define RL_LOG_INFO(a,b)  do { std::cerr << "[I] " << a << ": " << b << std::endl; } while(0)
#define RL_LOG_DEBUG(a,b) do { std::cerr << "[D] " << a << ": " << b << std::endl; } while(0)
#define RL_LOG_WARN(a,b)  do { std::cerr << "[W] " << a << ": " << b << std::endl; } while(0)
#define RL_LOG_ERROR(a,b) do { std::cerr << "[E] " << a << ": " << b << std::endl; } while(0)
#define RL_LOG_FATAL(a,b) do { std::cerr << "[F] " << a << ": " << b << std::endl; } while(0)


} // of namespace renderlist
} // of namespace tinia

