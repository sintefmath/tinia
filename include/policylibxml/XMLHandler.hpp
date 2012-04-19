#pragma once
#include "policylib/PolicyLib.hpp"
#include "policylibxml/XMLTransporter.hpp"
#include "policylibxml/XMLReader.hpp"
#include "policylibxml/ElementHandler.hpp"
#include <memory>
namespace policylibxml {
class XMLHandler
{
public:
   XMLHandler(std::shared_ptr<policylib::PolicyLib> policyLib);

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
   size_t getPolicyUpdate(char *buffer, const size_t buffer_len,
                          const unsigned int has_revision );

   xmlDocPtr getCompleteDocument();

private:
   std::shared_ptr<policylib::PolicyLib> m_policyLib;
   XMLTransporter m_xmlTransporter;
   XMLReader m_xmlReader;
   ElementHandler m_elementHandler;
};
}
