#include "tinia/policyxml/ElementHandler.hpp"

namespace tinia {
policyxml::ElementHandler::ElementHandler(std::shared_ptr<policy::Policy> policy)
   : m_policy(policy)
{
}

void policyxml::ElementHandler::updateElementFromString(const std::string& name,
                                                 const std::string& stringValue)
{
   m_policy->updateElementFromString(name, stringValue);
}

void policyxml::ElementHandler::updateElementFromPTree(const std::string &key,
                                                          const policy::StringStringPTree &tree)
{
   m_policy->updateElementFromPTree(key, tree);
}
}
