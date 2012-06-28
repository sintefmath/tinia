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

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>


/** \file utils.hpp
    This file contains various utilty function that is used throughout policy.
    They are mostly intended for internal-use, but might also be convenient for
    other applications where libxml is used.
  */


namespace tinia {
namespace policy {


template<typename U>
void print0(const boost::property_tree::basic_ptree<std::string, U> &pt, const int level) {
    //boost::property_tree::basic_ptree::const_iterator end = pt.end();
    auto end = pt.end();
    //boost::property_tree::basic_ptree<T, U>::const_iterator it = pt.begin();
    auto it = pt.begin();
    for ( ; it != end; ++it) {
        for (int i=0; i<level; i++)
            std::cout << " ";
        auto val = it->second;
        std::string valstr = "qwqw";
        //std::string valstr = it->second.get_value<std::string>();
        std::cout << it->first << ": " << valstr << std::endl;
        print0(it->second, level + 4);
    }
}

template<typename T, typename U>
void print(const boost::property_tree::basic_ptree<T, U> &pt) {
    std::cout << "------------- ptree -----------" << std::endl;
    print0(pt, 0);
    std::cout << "------------- ptree -----------" << std::endl;
}


}
}
#endif // UTILS_HPP






























