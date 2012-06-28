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
#include <string>
#include <mutex> 
#include "Observer.hpp"
#include "Job.hpp"
#include "tinia/model/ExposedModel.hpp"

namespace tinia {
namespace Trell
{

  class ComputeJob : public Job
{
public:
    ComputeJob( Observer* observer );

    virtual
    bool
    init( const std::string& xml ) = 0;

    virtual
    void
    run() = 0;


    virtual void actOnStateUpdate(std::vector<std::string> &updatedKeys) =0;
    
    virtual void cleanup();

    virtual void
    handleEvents();

  // Functions below this point should be called from the Observer
  void commitSuicide();

    void
    sendXMLStateToClient( char*               buffer,
                     const size_t        buffer_len,
			  size_t &document_length,
			  const unsigned int  has_revision ) ;//const;
    bool
    updateState( const char*   buffer,
                 const size_t  buffer_size );



protected:
  bool m_isSuicidal;
  int  m_LastStateRead;
  mutable std::mutex m_modelLock;
  model::ExposedModel m_model;


};



} // of namespace Trell
} // of namespace tinia
