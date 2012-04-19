#pragma once
#include <string>
#include <thread> 

namespace Trell
{

  class Observer;
  class ComputeJob;
  class ComputeThread
{
public:
  ComputeThread( Observer* observer, ComputeJob *job );

    bool
    init( const std::string& xml );

    void
    run();

  // Functions below this point should be called from the Observer
  void commitSuicide();

    void
    sendXMLStateToClient( char*               buffer,
                     const size_t        buffer_len,
			  size_t &document_length,
			  const unsigned int  has_revision ) ;//const;
    void
    updateState( const char*   buffer,
                 const size_t  buffer_size );



protected:
  bool m_isSuicidal;
  ComputeJob *m_job;
  std::thread m_computeThread;
};



} // of namespace Trell
