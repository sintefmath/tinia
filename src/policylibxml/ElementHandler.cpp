#include "policylibxml/ElementHandler.hpp"


policylibxml::ElementHandler::ElementHandler(std::shared_ptr<policylib::PolicyLib> policyLib)
   : m_policylib(policyLib)
{
}

void policylibxml::ElementHandler::updateElementFromString(const std::string& name,
                                                 const std::string& stringValue)
{
   m_policylib->updateElementFromString(name, stringValue);
}

void policylibxml::ElementHandler::updateElementFromPTree(const std::string &key,
                                                          const policylib::StringStringPTree &tree)
{
   m_policylib->updateElementFromPTree(key, tree);
}
