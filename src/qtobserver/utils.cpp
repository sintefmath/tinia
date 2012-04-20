#include "tinia/qtobserver/utils.hpp"

namespace tinia {
std::string qtobserver::prettyName(std::string key, std::shared_ptr<policy::Policy> policy)
{
   policy::StateSchemaElement element = policy->getStateSchemaElement(key);
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
