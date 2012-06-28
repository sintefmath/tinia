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

#include "trell/ComputeJob.hpp"
#include "model/ExposedModel.hpp"

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
  std::lock_guard<std::mutex> l(m_modelLock);
  document_length = m_model.getExposedModelUpdate(buffer, buffer_len, has_revision);
}

bool
ComputeJob::updateState( const char*   buffer,
			 const size_t  buffer_size )
{
  std::lock_guard<std::mutex> l(m_modelLock);
  std::vector<std::string> updatedKeys;
  m_model.updateState(buffer, doc_len, updatedKeys);
  document_length = m_model.getExposedModelUpdate(buffer, buffer_len, has_revision);
  m_isNotified = true;
}


} // of namespace Trell
