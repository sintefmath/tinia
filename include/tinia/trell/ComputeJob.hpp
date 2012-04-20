#pragma once
#include <string>
#include <mutex> 
#include "Observer.hpp"
#include "Job.hpp"
#include "tinia/policylib/PolicyLib.hpp"

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
  mutable std::mutex m_policyLock;
  policylib::PolicyLib m_policy;


};



} // of namespace Trell
} // of namespace tinia
