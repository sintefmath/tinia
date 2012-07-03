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
    class TypeException : public std::invalid_argument {
    public:
        TypeException(const std::string& actual, const std::string& expected)
            : std::invalid_argument(makeWhat(actual, expected)), m_actual(actual),
              m_expected(expected)
        {

        }

        ~TypeException() throw() {

        }

        std::string actual() {
            return m_actual;
        }

        std::string expected() {
            return m_expected;
        }

    private:
        std::string makeWhat(const std::string& actual, const std::string& expected) {
            std::stringstream ss;
            ss << "Types did not match. Expected: \"" << expected << "\", but got: \"" << actual<<"\".";
            return ss.str();
        }
        std::string m_actual;
        std::string m_expected;
    };
}}
