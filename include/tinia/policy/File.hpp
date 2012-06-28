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

#ifndef POLICY_FILE_HPP
#define POLICY_FILE_HPP

#include <iostream>
#include <sstream>

namespace tinia {
namespace policy {

class File
{
public:
    File();

    std::string fullPath() const;

    void fullPath(const std::string& fullPath);

    std::string name() const;

private:
   std::string m_fullPath;
};

std::ostream& operator<<(std::ostream& stream, const File& file);

std::istream& operator>>(std::istream& stream, File& file);
}
}
#endif // POLICY_FILE_HPP
