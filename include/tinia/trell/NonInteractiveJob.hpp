#pragma once
#include "Job.hpp"

namespace Trell
{

class NonInteractiveJob : public Job
{
public:
    NonInteractiveJob( Observer* observer );

  virtual
    bool
    renderFrame( const std::string&  session,
                 const std::string&  key,
                 const size_t        width,
                 const size_t        height );

  virtual
    size_t
    getPolicyUpdate( char*               buffer,
                     const size_t        buffer_len,
                     const unsigned int  has_revision );
  virtual
    bool
    updateState( const char*   buffer,
                 const size_t  buffer_size );

};

} // of namespace Trell
