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
