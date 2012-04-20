#pragma once
#include <string>
#include <memory>
#include "tinia/policylib/PolicyLib.hpp"
namespace policylibxml {
class ElementHandler
{
public:
   ElementHandler(std::shared_ptr<policylib::PolicyLib> policyLib);
   void updateElementFromString(const std::string& key,
                                const std::string& stringValue);
   void updateElementFromPTree(const std::string& key,
                               const policylib::StringStringPTree& tree);

private:
   std::shared_ptr<policylib::PolicyLib> m_policylib;
};
}

