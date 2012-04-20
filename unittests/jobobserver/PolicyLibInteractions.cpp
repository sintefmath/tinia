#include <boost/test/unit_test.hpp>
#include "tinia/jobobserver/Job.hpp"
#include <memory>
using namespace jobobserver;
class JobFixture : Job {
public:
   JobFixture() { policyLib = getPolicylib();}
   ~JobFixture() {}
   std::shared_ptr<policylib::PolicyLib> policyLib;
};

BOOST_FIXTURE_TEST_CASE(getFullStateTest, JobFixture)
{
   policyLib->addElement("Element1", 1);
   policyLib->addElement("Element2", 2);

   policyLib->addElement("Element3", 3);

   policyLib->addElement("Element4", 4);

   policyLib->addElement("Element5", 5);

   policyLib->addElement("Element6", 6);

   policyLib->addElement("Element7", 7);

   policyLib->addElement("Element8", 8);
   policyLib->addElement("Element9", 9);

   std::vector<policylib::StateSchemaElement> elements;
   policyLib->getFullStateSchema(elements);
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
