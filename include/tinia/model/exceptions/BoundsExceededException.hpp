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
#include <boost/lexical_cast.hpp>

namespace tinia { namespace model {
class BoundsExceededException : public std::invalid_argument {
public:
    BoundsExceededException(const std::string& value,
                            const std::string& min,
                            const std::string& max)
        : std::invalid_argument(makeWhat(value, min, max)),
          m_value(value), m_min(min), m_max(max)
    {
        ;
    }

    template<typename T>
    BoundsExceededException(T value, T min, T max)
        : std::invalid_argument(makeWhat(boost::lexical_cast<std::string>(value),
                                         boost::lexical_cast<std::string>(min),
                                         boost::lexical_cast<std::string>(max))),
          m_value(boost::lexical_cast<std::string>(value)),
          m_min(boost::lexical_cast<std::string>(min)),
          m_max(boost::lexical_cast<std::string>(max))
    {
        ;
    }

    // We need this to be complaint with invalid_argument (need to explicitly state
    // that we throw nothing)
    ~BoundsExceededException() throw() {
        // Do nothing
    }

    std::string maximum() {
        return m_max;
    }

    std::string minimum() {
        return m_min;
    }

    std::string value() {
        return m_value;
    }

private:
    std::string makeWhat(std::string value, std::string min, std::string max) {
        std::stringstream ss;
        ss << "The value \"" << value << "\" is not within the bounds [ " << min << ", " << max<< " ].";
        return ss.str();
    }

    std::string m_value;
    std::string m_min;
    std::string m_max;
};

namespace impl {

/** Handy utility function
 */
template<typename T>
void checkBounds(const T& value, const T& min, const T& max) {
    if (value < min || value > max || min > max) {
        throw BoundsExceededException(value, min, max);
    }
}
}
}}
