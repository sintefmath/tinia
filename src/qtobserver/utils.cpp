#include "tinia/qtobserver/utils.hpp"

namespace tinia {
std::string qtobserver::prettyName(std::string key, std::shared_ptr<policylib::PolicyLib> policyLib)
{
   policylib::StateSchemaElement element = policyLib->getStateSchemaElement(key);
   if(element.emptyAnnotation())
   {
      return key;
   }
   else
   {
      auto annotation =  element.getAnnotation();
      return annotation["en"];
   }

}

}
