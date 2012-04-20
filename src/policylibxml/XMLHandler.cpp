#include "tinia/policylibxml/XMLHandler.hpp"
#include "tinia/policylib/StateElement.hpp"
#include "tinia/policylib/StateSchemaElement.hpp"
#include "tinia/policylibxml/XMLBuilder.hpp"
#define XMLDEBUG {std::cerr<< __FILE__<<__LINE__ << std::endl;}
namespace policylibxml {
XMLHandler::XMLHandler(std::shared_ptr<policylib::PolicyLib> policyLib)
   : m_policyLib(policyLib), m_elementHandler(policyLib)
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


   std::vector<policylib::StateElement> stateElements;

   std::vector<policylib::StateSchemaElement> stateSchemaElements;

   m_policyLib->getFullStateSchema(stateSchemaElements);

   m_policyLib->getStateUpdate(stateElements, has_revision);

   if(stateElements.size() ==0)
   {
      return 0;
   }

   XMLBuilder builder(stateElements, stateSchemaElements, m_policyLib->getGUILayout(policylib::gui::DESKTOP),
                      m_policyLib->getRevisionNumber());

   XMLTransporter xmlTransporter;

   xmlDocPtr doc = builder.getDeltaDocument();

   const size_t bytes_written = xmlTransporter.writeXMLtoBuffer(doc, buffer, buffer_len);

   xmlFreeDoc(doc);

   return bytes_written;
}

}

xmlDocPtr policylibxml::XMLHandler::getCompleteDocument()
{
   std::vector<policylib::StateElement> stateElements;
   std::vector<policylib::StateSchemaElement> stateSchemaElements;
   m_policyLib->getFullStateSchema(stateSchemaElements);
   m_policyLib->getStateUpdate(stateElements, 0);


   XMLBuilder builder(stateElements, stateSchemaElements, m_policyLib->getGUILayout(policylib::gui::DESKTOP),
                      m_policyLib->getRevisionNumber());
   return builder.getDeltaDocument();

}
