#include "tinia/policyxml/XMLHandler.hpp"
#include "tinia/policy/StateElement.hpp"
#include "tinia/policy/StateSchemaElement.hpp"
#include "tinia/policyxml/XMLBuilder.hpp"
#define XMLDEBUG {std::cerr<< __FILE__<<__LINE__ << std::endl;}

namespace tinia {
namespace policyxml {
XMLHandler::XMLHandler(std::shared_ptr<policy::Policy> policy)
   : m_policy(policy), m_elementHandler(policy)
{
}

bool XMLHandler::updateState(const char *buffer, const size_t doc_len)
{
   xmlDocPtr doc = NULL;
   try {
      std::cerr<< std::string(buffer, doc_len)<<std::endl;

      doc = m_xmlTransporter.readXMLfromBuffer(buffer, doc_len);
      m_xmlReader.parseDocument(doc, m_elementHandler);
   } catch(const std::exception& e) {
      std::cerr<<"XML ERROR: \n";
      std::cerr<<std::string(buffer, doc_len)<<std::endl;
      std::cerr<<"MESSAGE: " << e.what() << std::endl;

   } catch(...) {
      std::cerr<<"UNKNOWN ERROR: \n";
      std::cerr<<std::string(buffer, doc_len)<<std::endl;
   }
   if(doc) {
            xmlFreeDoc(doc);
   }
   return true;
}

size_t XMLHandler::getPolicyUpdate(char *buffer, const size_t buffer_len,
                                   const unsigned int has_revision)
{


   std::vector<policy::StateElement> stateElements;

   std::vector<policy::StateSchemaElement> stateSchemaElements;

   m_policy->getFullStateSchema(stateSchemaElements);

   m_policy->getStateUpdate(stateElements, has_revision);

   if(stateElements.size() ==0)
   {
      return 0;
   }

   XMLBuilder builder(stateElements, stateSchemaElements, m_policy->getGUILayout(policy::gui::DESKTOP),
                      m_policy->getRevisionNumber());

   XMLTransporter xmlTransporter;

   xmlDocPtr doc = builder.getDeltaDocument();

   const size_t bytes_written = xmlTransporter.writeXMLtoBuffer(doc, buffer, buffer_len);

   xmlFreeDoc(doc);

   return bytes_written;
}

}

xmlDocPtr policyxml::XMLHandler::getCompleteDocument()
{
   std::vector<policy::StateElement> stateElements;
   std::vector<policy::StateSchemaElement> stateSchemaElements;
   m_policy->getFullStateSchema(stateSchemaElements);
   m_policy->getStateUpdate(stateElements, 0);


   XMLBuilder builder(stateElements, stateSchemaElements, m_policy->getGUILayout(policy::gui::DESKTOP),
                      m_policy->getRevisionNumber());
   return builder.getDeltaDocument();

}
}
