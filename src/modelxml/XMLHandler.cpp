/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

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
