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

#include "tinia/model/File.hpp"

namespace tinia {
namespace model {

File::File() : m_fullPath("")
{
}



std::string File::fullPath() const
{
   return m_fullPath;
}

void File::fullPath(const std::string& fullPath)
{
   m_fullPath = fullPath;
}

std::string File::name() const
{
   // broken, for now
   return m_fullPath;
}


std::ostream& operator<<(std::ostream& stream, const File& file)
{
   stream << file.fullPath();
   return stream;
}

std::istream& operator>>(std::istream& stream, File& file)
{
   std::string fullPath;

   stream >> fullPath;
   file.fullPath(fullPath);
   stream.clear();
   return stream;

}

}
} // of namespace tinia
