#pragma once

#include <stdexcept>

#ifdef __unix
#include <sstream>
#include <execinfo.h>
#endif

namespace siut2 {
    namespace exceptions {

/** A wrapper for runtime_exception that includes the backtrace.
  *
  * This wrapper extends the runtime_exception class by adding the backtrace of
  * where the exeption was thrown to the message string.
  *
  * \note Currently only implemented on Linux.
  *
  * \author Christopher Dyken, <christopher.dyken@sintef.no>
  */
class backtrace_exception : public std::runtime_error
{
public:
    backtrace_exception( const std::string& message )
    : runtime_error( addBackTrace( message ) )
    {}

protected:

    std::string
    addBackTrace( const std::string& message )
    {
#ifdef __unix
        std::stringstream msg;
        msg << message << "\n";
        msg << "Backtrace:\n";

        void* buffer[64];
        int n = backtrace( buffer, 64 );

        char ** symbols = backtrace_symbols( buffer, n );
        for( int i=0; i<n; i++ ) {
            msg << i << ": " << symbols[i]<< "\n";
        }
        return msg.str();
#else
        return message;
#endif
    }

};

    } // namespace exceptions
} // namespace siut


