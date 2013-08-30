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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <vector>
#include <iostream>
#include <cstring>
#include "tinia/trell/OffscreenGL.hpp"

namespace tinia {
namespace trell {
namespace impl {

bool OffscreenGL::m_create_context_error = false;

int
OffscreenGL::createContextErrorHandler( Display* display, XErrorEvent* error )
{
    if( error->error_code != BadMatch ) {
        // Unexpected error!
        std::cerr << "OffscreenGL: Unexpected X error: " << (int)(error->error_code) << "\n";
    }
    m_create_context_error = true;
    return 0;
}


OffscreenGL::OffscreenGL( const std::string& display_string, int screen_number )
    : m_state( STATE_UNINITIALIZED ),
      m_display_string( display_string ),
      m_screen_number( screen_number ),
      m_display( NULL ),
      m_context( NULL ),
      m_req_profile(false),
      m_req_debug( false )
{
    
}

bool
OffscreenGL::setupContext()
{
    XVisualInfo* vis = NULL;
    GLXFBConfig* glx_fb_configs = NULL;
    const char* glx_extensions = NULL;
    bool has_glx_arb_create_context_profile = false;

    if( m_display_string.empty() ) {
        const char* env_display = getenv( "DISPLAY" );
        if( env_display != NULL ) {
            m_display_string = env_display;
        }
        else {
            m_display_string = ":0.0";
        }
    }
    
    m_display = XOpenDisplay( m_display_string.c_str() );
    if( m_display == NULL ) {
        std::cerr << __FILE__ << "@" << __LINE__ << "\n";
        m_state = STATE_FAILED_TO_OPEN_DISPLAY;
        goto cleanup;
    }
    
    if( ScreenCount(m_display) < 1 ) {
        std::cerr << __FILE__ << "@" << __LINE__ << "\n";
        m_state = STATE_NO_SCREENS;
        goto cleanup;
    }
    if( (m_screen_number < 0) || (m_screen_number >= ScreenCount(m_display) ) ) {
        m_screen_number = DefaultScreen( m_display );
    } 
    
    // We require at least GLX 1.3
    int glx_major, glx_minor;
    if( (glXQueryVersion( m_display, &glx_major, &glx_minor) != True )
            || (glx_major < 1) || (glx_minor < 3) )
    {
        std::cerr << __FILE__ << "@" << __LINE__ << "\n";
        m_state = STATE_INSUFFICIENT_GLX;
        goto cleanup;
    }
    glx_extensions = glXQueryExtensionsString( m_display, m_screen_number );
    
    has_glx_arb_create_context_profile = strstr( glx_extensions, "GLX_ARB_create_context_profile" ) != NULL;

    // --- try making context using a fb config --------------------------------
    for( int i=0; (i<2) && (glx_fb_configs==NULL); i++ ) {
        int glx_fb_configs_N;
        int fb_attrib[] = {
            GLX_RENDER_TYPE,    GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
            GLX_RED_SIZE,       1,
            GLX_GREEN_SIZE,     1,
            GLX_BLUE_SIZE,      1,
            (i==0?(int)None:(int)GLX_DOUBLEBUFFER),
            None
        };
        glx_fb_configs = glXChooseFBConfig( m_display,
                                            m_screen_number,
                                            fb_attrib,
                                            &glx_fb_configs_N );
    }

    // --- create context from fb config ---------------------------------------
    if( glx_fb_configs != NULL ) {
#ifdef GLX_VERSION_1_4
#ifdef GLX_ARB_create_context_profile
        // --- try to use glXCreateContextAttribs ------------------------------
        if( (glx_minor >= 4) && has_glx_arb_create_context_profile ) {
            
            
            PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB_f = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddressARB((GLubyte *) "glXCreateContextAttribsARB");
            if( glXCreateContextAttribsARB_f != NULL ) {

                std::vector<int> attribs;
                if( m_req_profile ) {
                    attribs.push_back( GLX_CONTEXT_MAJOR_VERSION_ARB );
                    attribs.push_back( m_req_profile_args.m_major );
                    attribs.push_back( GLX_CONTEXT_MINOR_VERSION_ARB );
                    attribs.push_back( m_req_profile_args.m_minor );
                    attribs.push_back( (m_req_profile_args.m_core ? GLX_CONTEXT_CORE_PROFILE_BIT_ARB : 0 ) |
                                       (m_req_profile_args.m_compatibility ? GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : 0 ) );
                }
                if( m_req_debug ) {
                    //attribs.push_back( GLX_CONTEXT_PROFILE_MASK_ARB );
                    //attribs.push_back( GLX_CONTEXT_DEBUG_BIT_ARB);
                }
                attribs.push_back( None );

                // Try direct and not direct
                for(int k=0; k<2 && (m_context == NULL); k++ ) {
                    m_create_context_error = false;
                    
                    // Illegal GL versions create X-errors. Make
                    // them silent and simply flag that we failed.
                    int (*old)(Display *, XErrorEvent *);
                    old = XSetErrorHandler( createContextErrorHandler );
                    m_context = glXCreateContextAttribsARB_f( m_display,
                                                              glx_fb_configs[0],
                                                              NULL,
                                                              (k==0?True:False),
                                                              attribs.data() );
                    XSetErrorHandler( old );
                    if( m_context != NULL ) {
                        if( m_create_context_error ) {
                            // got error creating context, not trusting it.
                            glXDestroyContext( m_display, m_context );
                            m_context = NULL;
                        }
                        else {
                            std::cerr << "OffscreenGL: glXCreateContextAttribs (k="<<k<<") succeeded.\n";
                        }
                    }
                }
                
            }
        }
#endif
#endif
        // --- try to use glXCreateNewContext ----------------------------------
        for(int k=0; k<2 && (m_context == NULL); k++ ) {
            m_context = glXCreateNewContext( m_display,
                                             glx_fb_configs[0],
                                             GLX_RGBA_TYPE,
                                             NULL,
                                             (k==0?True:False) );
            if( m_context != NULL ) {
                std::cerr << "OffscreenGL: glXCreateNewContext (k="<<k<<") succeeded.\n";
            }
        }
        vis = glXGetVisualFromFBConfig( m_display, glx_fb_configs[0] );
    }

    // --- create context from visual info -------------------------------------
    if( m_context == NULL ) {
        
        // create visual info, first single-buffer, then double-buffer
        for(int i=0; (i<2) && (vis==NULL); i++ ) {
            int attribs[] = {
                GLX_RGBA,
                GLX_RED_SIZE, 1,
                GLX_GREEN_SIZE, 1,
                GLX_BLUE_SIZE, 1,
                (i==0?(int)None:(int)GLX_DOUBLEBUFFER),
                None
            };
            vis = glXChooseVisual( m_display, m_screen_number, attribs );
        }
        
        if( vis != NULL ) {
            // Try first direct, then indirect.
            for( int k=0; (k<2)&& (m_context == NULL ); k++ ) {
                m_context = glXCreateContext( m_display, vis, NULL, k==0?True:False );
                if( m_context != NULL ) {
                    std::cerr << "OffscreenGL: glXCreateContext (k="<<k<<") succeeded.\n";
                }
            } 
        }
        
    }
        
    if( m_context == NULL ) {
        std::cerr << __FILE__ << "@" << __LINE__ << "\n";
        m_state = STATE_X_ERROR;
        goto cleanup;
    }
    
    if( vis == NULL ) {
        std::cerr << __FILE__ << "@" << __LINE__ << "\n";
        m_state = STATE_X_ERROR;
        goto cleanup;
    }

    // --- Create a window to use with our context -----------------------------
    if( vis == NULL ) {
        m_state = STATE_X_ERROR;
        goto cleanup;
    }

    XSetWindowAttributes swa;
    memset( &swa, 0, sizeof(XSetWindowAttributes) );
    swa.border_pixel = 0;
    swa.event_mask = StructureNotifyMask;
    swa.colormap = XCreateColormap( m_display,
                                    RootWindow( m_display, m_screen_number ),
                                    vis->visual,
                                    AllocNone );

    m_window = XCreateWindow( m_display,
                              RootWindow( m_display, m_screen_number ),
                              0, 0, 32, 32, 0, vis->depth,
                              InputOutput,
                              vis->visual,
                              CWBorderPixel | CWColormap | CWEventMask,
                              &swa );
    
    m_state = STATE_INITIALIZED;

cleanup:
    if( vis != NULL ) {
        XFree( vis );
    }
    if( glx_fb_configs != NULL ) {
        XFree( glx_fb_configs );
    }
    return m_state == STATE_INITIALIZED;
}

OffscreenGL::~OffscreenGL()
{
    //XDestroyWindow( m_display, m_window );
    
    if( m_context != NULL ) {
        glXDestroyContext( m_display, m_context );
    }
    if( m_display != NULL ) {
        XCloseDisplay( m_display );
    }
}


bool
OffscreenGL::bindContext()
{
    if( m_state >= STATE_INITIALIZED ) {
        if( glXMakeCurrent( m_display, m_window, m_context ) ) {
            m_state = STATE_CONTEXT_BOUND;
        }
        
    }
    
    return true;    
}

void
OffscreenGL::requestProfile( int major, int minor, bool core, bool compatibility )
{
    m_req_profile_args.m_major = major;
    m_req_profile_args.m_minor = minor;
    m_req_profile_args.m_core = core;
    m_req_profile_args.m_compatibility = compatibility;
    m_req_profile = true;
}


} // of namespace impl
} // of namespace trell
} // of namespace tinia
