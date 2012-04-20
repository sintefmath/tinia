#include "trell/ComputeJob.hpp"
#include "policy/Policy.hpp"

namespace Trell
{

ComputeJob::ComputeJob( Observer* observer ) : 
  Job(observer), m_isSuicidal(false), m_LastStateRead(-1)
{
  ;
}



  // Functions below this point should be called from the Observer
void 
ComputeJob::commitSuicide()
{
  m_isSuicidal=true;
}

void
ComputeJob::sendXMLStateToClient( char*               buffer,
				  const size_t        buffer_len,
				  size_t &document_length,
				  const unsigned int  has_revision ) // const
{
  std::lock_guard<std::mutex> l(m_policyLock);
  document_length = m_policy.getPolicyUpdate(buffer, buffer_len, has_revision);
}

bool
ComputeJob::updateState( const char*   buffer,
			 const size_t  buffer_size )
{
  std::lock_guard<std::mutex> l(m_policyLock);
  std::vector<std::string> updatedKeys;
  m_policy.updateState(buffer, doc_len, updatedKeys);
  document_length = m_policy.getPolicyUpdate(buffer, buffer_len, has_revision);
  m_isNotified = true;
}


} // of namespace Trell
