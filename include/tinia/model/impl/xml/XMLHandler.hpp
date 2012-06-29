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

#pragma once
#include "tinia/model/ExposedModel.hpp"
#include "tinia/model/impl/xml/XMLTransporter.hpp"
#include "tinia/model/impl/xml/XMLReader.hpp"
#include "tinia/model/impl/xml/ElementHandler.hpp"
#include <memory>

namespace tinia {
namespace model {
namespace impl {
namespace xml {

class XMLHandler
{
public:
   XMLHandler(std::shared_ptr<model::ExposedModel> model);

   /** The job can use this to update the state given new information from the client.
      \param buffer The memory buffer to which the xml document will be written.
      \param doc_len The size of the xml document in the buffer.
      \return True if everything is ok, otherwise false.
      */
   bool updateState(const char *buffer, const size_t doc_len);

   /** The job can use this to get an update meant for the client.
      \param buffer The memory buffer to which the xml document will be written.
      \param buffer_len The size of the buffer.
      \param has_revision Base number from which the delta is to be computed. The client needs updates for revisions after this.
      \return The number of bytes actually written. Zero if there was no update at all.
      */
   size_t getExposedModelUpdate(char *buffer, const size_t buffer_len,
                          const unsigned int has_revision );

   xmlDocPtr getCompleteDocument();

private:
   std::shared_ptr<model::ExposedModel> m_model;
   XMLTransporter m_xmlTransporter;
   XMLReader m_xmlReader;
   ElementHandler m_elementHandler;
};
}
}
}
}
