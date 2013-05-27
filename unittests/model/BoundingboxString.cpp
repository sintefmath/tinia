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


#include <boost/test/unit_test.hpp>
#include <tinia/model.hpp>

BOOST_AUTO_TEST_SUITE(BoundingBox)
BOOST_AUTO_TEST_CASE(BoundingBoxString) {
	std::string bb = tinia::model::makeBoundingBoxString(0, 0, 0, 1, 1, 1);
    BOOST_CHECK_EQUAL("0 0 0 1 1 1", bb);
}
BOOST_AUTO_TEST_SUITE_END()
