/* Copyright STIFTELSEN SINTEF 2013
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
#include <iostream>

#include <sstream>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <map>
#include "Applications.hpp"

namespace tinia {
namespace trell {
namespace impl {

Applications::Applications(const std::string &app_root)
    : m_state( STATE_OK ),
      m_timestamp( 1 ),
      m_app_root( app_root ),
      m_last_scan( 0 )
{
    refresh();
}

void
Applications::refresh( )
{
    bool change = false;
    typedef std::list<Application>::iterator ALIT;
    typedef std::map<std::string, ALIT>::iterator AMIT;

    // Check if directory directory has been modified, if so, we do a full
    // rescan.
    m_state = STATE_OK;
    struct stat stat_buf;
    if( stat( m_app_root.c_str(), &stat_buf ) != 0 ) {
        std::cerr << "stat(" << m_app_root << "): " << strerror( errno ) << "\n";
        // Failed to stat the dir
        switch( errno ) {
        case ENAMETOOLONG:
        case ENOTDIR:
        case ENOENT:
            m_state = STATE_ILLEGAL_PATH;
            break;
        case EACCES:
            m_state = STATE_ACCESS_DENIED;
            break;
        default:
            m_state = STATE_INTERNAL_ERROR;
            break;
        }
        m_applications.clear();
        change = true;
    }
    else if( !S_ISDIR( stat_buf.st_mode ) ) {
        std::cerr << m_app_root << " is not a directory.\n";
        m_state = STATE_ILLEGAL_PATH;
        m_applications.clear();
        change = true;
    }
    else if( m_last_scan == stat_buf.st_mtime ) {
        // No modification!
        // If you have changed permissions on an application, do a touch
        // on the application path to force a refresh.
        change = false;
    }
    else {
        
        DIR* app_dir = opendir( m_app_root.c_str() );
        if( app_dir == NULL ) {
            std::cerr << "opendir(" << m_app_root << "): " << strerror( errno ) << "\n";
            switch( errno ) {
            case EACCES:
                m_state = STATE_ACCESS_DENIED;
                break;
            default:
                m_state = STATE_INTERNAL_ERROR;
                break;
            }
            m_applications.clear();
        }
        else {
            m_last_scan = stat_buf.st_mtime;

            
            // Get list of currently registered applications.
            std::map<std::string, ALIT> kill_list;
            for( ALIT it=m_applications.begin(); it != m_applications.end(); ++it ) {
                kill_list.insert( std::pair<std::string,ALIT>( it->appBinary(), it ) );
            }

            // Loop over files in directory.
            while( 1 ) {
                struct dirent* dir_entry = readdir( app_dir );
                if( dir_entry == NULL ) {
                    break;
                }
                std::string app_bin = dir_entry->d_name;
                std::string app_path = m_app_root + "/" + app_bin;
                
                // Check if it is a regular file.
                if( stat( app_path.c_str(), &stat_buf ) != 0 ) {
                    std::cerr << "stat(" << app_path << "): " << strerror( errno ) << "\n";
                }
                else if( S_ISREG( stat_buf.st_mode ) ) {
                    // Check that we have execute permissions
                    if( access( app_path.c_str(), X_OK ) == 0 ) {
                        AMIT it = kill_list.find( app_bin );
                        if( it != kill_list.end() ) {
                            // already have it
                            kill_list.erase( it );
                        }
                        else {
                            // found new application.
                            m_applications.push_back( Application( app_bin, app_path ) );
                        }
                    }
                }
            }
            
            for( AMIT it = kill_list.begin(); it!=kill_list.end(); ++it ) {
                m_applications.erase( it->second );
            }
        }
        change = true;
    }
    
    for( ALIT it=m_applications.begin(); it != m_applications.end(); ++it ) {
        if( it->refresh() ) {
            change = true;
        }
    }
    
    if( change ) {
        m_timestamp++;
    }
    
}

const std::string
Applications::xml() const
{
    std::stringstream xml;
    xml << "  <applications>\n";
    switch (m_state) {
    case STATE_OK:
        for( std::list<Application>::const_iterator it=m_applications.begin(); it != m_applications.end(); ++it ) {
            xml << it->xml();
        }
        break;
    case STATE_ACCESS_DENIED:
        xml << "    <error>ACCESS_DENIED</error>\n";
        break;
    case STATE_ILLEGAL_PATH:
        xml << "    <error>ILLEGAL_PATH</error>\n";
        break;
    case STATE_INTERNAL_ERROR:
        xml << "    <error>INTERNAL_ERROR</error>\n";
        break;
    default:
        break;
    }    
    xml << "  </applications>\n";
    return xml.str();
}

} // of namespace impl
} // of namespace trell
} // of namespace tinia
