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
#include<string>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

namespace tinia {
namespace trell {
namespace impl {

class OffscreenGL
{
public:
    enum ObjectState {
        STATE_UNINITIALIZED,
        STATE_FAILED_TO_OPEN_DISPLAY,
        STATE_NO_SCREENS,
        STATE_INSUFFICIENT_GLX,
        STATE_X_ERROR,
        STATE_INITIALIZED,
        STATE_CONTEXT_BOUND
    };
    
    OffscreenGL( void (*logger)( void* data, int level, const char* who, const char* message, ... ) = NULL,
                 void* logger_data = NULL );

    ~OffscreenGL();

    void
    requestProfile( int major, int minor, bool core, bool compatibility );
    
    void
    requestDebug();
    
    bool
    setupContext( const std::string& display_string );
    
    bool
    bindContext();

    ObjectState
    state() const { return m_state; }
    
    Display*
    display() { return m_display; }
    
    const std::string&
    displayName() { return m_display_string; }

    GLXContext
    context() { return m_context; }
    
    int
    screenNumber() { return m_screen_number; }
    
protected:
    ObjectState m_state;
    std::string m_display_string;
    void      (*m_logger)( void* data, int level, const char* who, const char* message, ... );
    void*       m_logger_data;
    std::string m_logger_who;
    
    int         m_screen_number;
    Display*    m_display;
    GLXContext  m_context;
    Window      m_window;
    
    bool        m_req_profile;
    bool        m_req_debug;

    struct {
        int     m_major;
        int     m_minor;
        bool    m_core;
        bool    m_compatibility;
    }           m_req_profile_args;

    static bool                 m_create_context_error;
    
    static
    int
    createContextErrorHandler( Display* display, XErrorEvent* error );
    
};

} // of namespace impl
} // of namespace trell
} // of namespace tinia
