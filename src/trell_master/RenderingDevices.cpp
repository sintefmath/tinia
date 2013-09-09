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
#define GLX_GLXEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <dirent.h>
#include "RenderingDevices.hpp"
#include "tinia/trell/OffscreenGL.hpp"

namespace tinia {
namespace trell {
namespace impl {

static const std::string package = "RenderingDevices";

RenderingDevices::RenderingDevices(void (*logger)( void* data, int level, const char* who, const char* message, ... ),
                                   void* logger_data )
    : m_display_name( ":0.0" ),
      m_logger( logger ),
      m_logger_data( logger_data )
{
}

std::list<std::string>
RenderingDevices::parseExtensions( const char* string )
{
    std::list<std::string> list;
    if( (string != NULL) ) {
        const char* a = string;
        while( *a != '\0' ) {
            while( (*a == ' ' ) ) { a++; }
            const char* b = a;
            while( (*b != ' ' ) && (*b!='\0') ) { b++; }
            if( a != b ) {
                list.push_back( std::string( a, b ) );
            }
            a = b;
        }
    }
    return list;
}


std::string
RenderingDevices::xml()
{

    std::stringstream xml;
    xml << "  <renderingDevices>\n";
   
    DIR* dir = opendir( "/tmp/.X11-unix" );
    if( dir == NULL ) {
        if( m_logger != NULL ) {
            m_logger( m_logger_data, 0, package.c_str(),
                      "opendir failed: %s.", strerror( errno ) );
        }
        xml << "    <error>INTERNAL_ERROR</error>\n";
    }
    else {
        struct dirent* entry = NULL;
        while( (entry = readdir( dir )) != NULL ) {
            if( entry->d_name[0] != 'X' ) {
                continue;
            }
            std::string display_id = ":" + std::string( entry->d_name ).substr(1);
    
    
            
            Display* display = XOpenDisplay( display_id.c_str() );
            if( display == NULL ) {
                xml << "    <renderingDevice id=\"" << display_id << "\">\n";
                xml << "      <error>FAILED_TO_OPEN_DISPLAY</error>\n";
                xml << "    </renderingDevice>\n";
                continue;
            }

            int screen_count = ScreenCount( display );
            XCloseDisplay( display );
            
            if( screen_count < 1 ) {
                xml << "    <renderingDevice id=\"" << display_id << "\">\n";
                xml << "      <error>NO_SCREENS</error>\n";
                xml << "    </renderingDevice>\n";
                continue;
            }

            for(int screen=0; screen<screen_count; screen++) {
                std::stringstream screen_id_ss;
                screen_id_ss << display_id << '.' << screen;
                std::string screen_id = screen_id_ss.str();
                
                xml << "    <renderingDevice id=\"" << screen_id << "\">\n";
                OffscreenGL gl( screen_id, m_logger, m_logger_data );
                if( !gl.setupContext() || !gl.bindContext() ) {
                    switch ( gl.state() ) {
                    case OffscreenGL::STATE_INSUFFICIENT_GLX:
                        xml << "    <error>INSUFFICIENT_GLX</error>\n";
                        break;
                    case OffscreenGL::STATE_X_ERROR:
                        xml << "    <error>X_ERROR</error>\n";
                        break;
                    default:
                        xml << "    <error>INTERNAL_ERROR</error>\n";
                        break;
                    }
                }
                else {
                    
                    int glx_major = 0;
                    int glx_minor = 0;
                    glXQueryVersion( gl.display(), &glx_major, &glx_minor );
                    xml << "      <glx major=\"" << glx_major
                        << "\" minor=\"" << glx_minor
                        << "\" direct=\"" << (glXIsDirect(gl.display(), gl.context())==True?'1':'0')
                        << "\">\n";
                    xml << "        <client>\n";
                    xml << "          <vendor>" << glXGetClientString( gl.display(), GLX_VENDOR ) << "</vendor>\n";
                    xml << "          <version>" << glXGetClientString( gl.display(), GLX_VERSION ) << "</version>\n";
                    xml << "        </client>\n";
                    xml << "        <server>\n";
                    xml << "          <vendor>" << glXQueryServerString( gl.display(), screen, GLX_VENDOR ) << "</vendor>\n";
                    xml << "          <version>" << glXQueryServerString( gl.display(), screen, GLX_VERSION ) << "</version>\n";
                    xml << "        </server>\n";
                    std::list<std::string> glx_extensions = parseExtensions( glXQueryExtensionsString( gl.display(), screen ) );
                    for( std::list<std::string>::iterator it=glx_extensions.begin(); it!=glx_extensions.end(); ++it ) {
                        //xml << "        <extension>" << *it << "</extension>\n";
                    }
                    xml << "      </glx>\n";
                    
                    GLint gl_major = 0;
                    glGetIntegerv( GL_MAJOR_VERSION, &gl_major );
                    GLint gl_minor = 0;
                    glGetIntegerv( GL_MINOR_VERSION, &gl_minor );
                    xml << "      <opengl major=\"" << gl_major
                        << "\" minor=\"" << gl_minor
                        << "\">\n";
                    xml << "        <vendor>" << (const char*)glGetString( GL_VENDOR ) << "</vendor>\n";
                    xml << "        <version>" << (const char*)glGetString( GL_VERSION ) << "</version>\n";
                    xml << "        <renderer>" << (const char*)glGetString( GL_RENDERER) << "</renderer>\n";
                    
                    if( gl_major >= 2 ) {
                        xml << "        <glsl><version>"
                            << (const char*)glGetString( GL_SHADING_LANGUAGE_VERSION )
                            << "</version></glsl>\n";
                        
                    }
                    std::list<std::string> gl_extensions = parseExtensions( (const char*)glGetString( GL_EXTENSIONS ) );
                    for( std::list<std::string>::iterator it=gl_extensions.begin(); it!=gl_extensions.end(); ++it ) {
                        //xml << "        <extension>" << *it << "</extension>\n";
                    }
                    xml << "      </opengl>\n";
                }
                xml << "    </renderingDevice>\n";
            }
        }
        closedir( dir );
    }
    xml << "  </renderingDevices>\n";
    return xml.str();
}

} // of namespace impl
} // of namespace trell
} // of namespace tinia
