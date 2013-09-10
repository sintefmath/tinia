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
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>
#include "tinia/jobcontroller/Controller.hpp"
namespace tinia {
namespace jobcontroller {
namespace {


/** Default logger that outputs log messages to stderr. */
static void defaultLogger( void* data,
                           int level,
                           const char* who,
                           const char* msg,
                           ... )
{
    char buf[1024];
    va_list args;
    va_start( args, msg );
    vsnprintf( buf, sizeof(buf), msg, args );
    va_end( args );
    switch( level ) {
    case 0: std::cerr << "[E] "; break;
    case 1: std::cerr << "[W] "; break;
    default:  std::cerr << "[I] "; break;
    }
    if( who != NULL ) {
        std::cerr << '[' << who << "]";
    }
    for( int i = strlen(who); i<30; i++ ) {
        std::cerr << ' ';
    }
    std::cerr << buf << std::endl;    
}

} // of empty namespace


Controller::Controller()
    : m_logger_callback( defaultLogger ),
      m_logger_data( NULL )
{
}

}
}
