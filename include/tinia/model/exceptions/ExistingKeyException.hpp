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
#include <stdexcept>
namespace tinia { namespace model {
class ExistingKeyException : public std::invalid_argument {
public:
    ExistingKeyException(const std::string& key)
        : std::invalid_argument(makeWhat(key)), m_key(key)
    {

    }

    // We need this to be complaint with invalid_argument (need to explicitly state
    // that we throw nothing)
    ~ExistingKeyException() throw() {
        // Do nothing;
    }

    const std::string& key() const {
        return m_key;
    }

private:
    std::string makeWhat(const std::string& key) {
        std::stringstream ss;
        ss << "The key \"" << key << "\" is already defined in the model.";
        return ss.str();
    }

    std::string m_key;
};
}}
