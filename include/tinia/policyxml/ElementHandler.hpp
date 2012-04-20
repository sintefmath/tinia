#pragma once
#include <string>
#include <memory>
#include "tinia/policy/Policy.hpp"
namespace tinia {
namespace policyxml {
class ElementHandler
{
public:
   ElementHandler(std::shared_ptr<policy::Policy> policy);
   void updateElementFromString(const std::string& key,
                                const std::string& stringValue);
   void updateElementFromPTree(const std::string& key,
                               const policy::StringStringPTree& tree);

private:
   std::shared_ptr<policy::Policy> m_policy;
};
}
}
