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

#include "tinia/modelxml/XMLHandler.hpp"
#include "tinia/model/StateElement.hpp"
#include "tinia/model/StateSchemaElement.hpp"
#include "tinia/modelxml/XMLBuilder.hpp"
#define XMLDEBUG {std::cerr<< __FILE__<<__LINE__ << std::endl;}

namespace tinia {
namespace modelxml {
XMLHandler::XMLHandler(std::shared_ptr<model::ExposedModel> model)
   : m_model(model), m_elementHandler(model)
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

size_t XMLHandler::getExposedModelUpdate(char *buffer, const size_t buffer_len,
                                   const unsigned int has_revision)
{


   std::vector<model::StateElement> stateElements;

   std::vector<model::StateSchemaElement> stateSchemaElements;

   m_model->getFullStateSchema(stateSchemaElements);

   m_model->getStateUpdate(stateElements, has_revision);

   if(stateElements.size() ==0)
   {
      return 0;
   }

   XMLBuilder builder(stateElements, stateSchemaElements, m_model->getGUILayout(model::gui::DESKTOP),
                      m_model->getRevisionNumber());

   XMLTransporter xmlTransporter;

   xmlDocPtr doc = builder.getDeltaDocument();

   const size_t bytes_written = xmlTransporter.writeXMLtoBuffer(doc, buffer, buffer_len);

   xmlFreeDoc(doc);

   return bytes_written;
}

}

xmlDocPtr modelxml::XMLHandler::getCompleteDocument()
{
   std::vector<model::StateElement> stateElements;
   std::vector<model::StateSchemaElement> stateSchemaElements;
   m_model->getFullStateSchema(stateSchemaElements);
   m_model->getStateUpdate(stateElements, 0);


   XMLBuilder builder(stateElements, stateSchemaElements, m_model->getGUILayout(model::gui::DESKTOP),
                      m_model->getRevisionNumber());
   return builder.getDeltaDocument();

}
}
