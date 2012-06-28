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


