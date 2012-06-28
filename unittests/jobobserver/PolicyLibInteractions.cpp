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
#include "tinia/jobobserver/Job.hpp"
#include <memory>
using namespace tinia::jobobserver;
class JobFixture : Job {
public:
   JobFixture() { policy = getPolicy();}
   ~JobFixture() {}
   std::shared_ptr<tinia::policy::Policy> policy;
};

BOOST_FIXTURE_TEST_CASE(getFullStateTest, JobFixture)
{
   policy->addElement("Element1", 1);
   policy->addElement("Element2", 2);

   policy->addElement("Element3", 3);

   policy->addElement("Element4", 4);

   policy->addElement("Element5", 5);

   policy->addElement("Element6", 6);

   policy->addElement("Element7", 7);

   policy->addElement("Element8", 8);
   policy->addElement("Element9", 9);

   std::vector<tinia::policy::StateSchemaElement> elements;
   policy->getFullStateSchema(elements);
   std::vector<bool> found;

   for(int i = 0; i < 9; i++) found.push_back(false);
   for(auto it = elements.begin(); it!=elements.end(); it++)
   {
      std::string key = it->getKey();
      int i = (int)(key[key.size()-1]-'0');
      BOOST_CHECK( i > 0);
      BOOST_CHECK( i <= 9);
      found[i-1] = true;
   }
   std::vector<bool>foundCheck;
   for(int i = 0; i < 9; i++) foundCheck.push_back(true);
   BOOST_CHECK_EQUAL_COLLECTIONS(foundCheck.begin(), foundCheck.end(),
                                 found.begin(), found.end());
}
