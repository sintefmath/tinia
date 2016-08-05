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

#include <cstdlib>      // getenv
#include <sstream>
#include <tinia/renderlist/XMLWriter.hpp>
#include "tinia/trell/IPCGLJobController.hpp"

namespace {

static const std::string package = "IPCGLJobController";

struct GLDebugLogWrapperData
{
    void  (*m_logger_callback)( void* logger_data, int level, const char* who, const char* msg, ... );
    void*   m_logger_data;
};


#ifdef GLEW_khr_DEBUG // make sure the glew version is new enough
static
void
GLDebugLogWrapper( GLenum source,
                   GLenum type,
                   GLuint id,
                   GLenum severity,
                   GLsizei length,
                   const GLchar* message,
                   void* data )
{

    const char* source_str = "???";
    switch( source ) {
    case GL_DEBUG_SOURCE_API:             source_str = "api"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   source_str = "wsy"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: source_str = "cmp"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     source_str = "3py"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     source_str = "app"; break;
    case GL_DEBUG_SOURCE_OTHER:           source_str = "oth"; break;
    default: break;
    }

    const char* type_str = "???";
    switch( type ) {
    case GL_DEBUG_TYPE_ERROR:               type_str = "error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_str = "deprecated"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  type_str = "undef"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         type_str = "portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         type_str = "performance"; break;
    case GL_DEBUG_TYPE_OTHER:               type_str = "other"; break;
    default: break;
    }

    int level = 2;
    switch( severity ) {
    case GL_DEBUG_SEVERITY_HIGH:   level = 0; break;
    case GL_DEBUG_SEVERITY_MEDIUM: level = 1; break;
    default: break;
    }

    GLDebugLogWrapperData* d = reinterpret_cast<GLDebugLogWrapperData*>( data );
    d->m_logger_callback( d->m_logger_data,
                          level,
                          "OpenGL",
                          "src=%s;type=%s: %s",
                          source_str,
                          type_str,
                          message );
}
#endif

} // of anonymous namespace


namespace tinia {
namespace trell {


IPCGLJobController::IPCGLJobController(bool is_master)
    : IPCJobController( is_master ),
      m_openGLJob( NULL ),
      m_context( m_logger_callback, m_logger_data ),
      m_quality( 0 )
{
}

void
IPCGLJobController::setQuality( int quality )
{
    m_quality = std::max( 0, std::min( 255, quality ) );
}

bool
IPCGLJobController::init()
{
    // Initialize this
    m_openGLJob = static_cast<jobcontroller::OpenGLJob*>(m_job);

    // How should we let the job enable debugging? Or specific GL versions?
    bool debug = true;

    
    // --- set up OpenGL context ------------------------------------------------
    // The display to use is passed from the master job via an env-var.
    std::string displays;
    {
        const char* t = getenv( "TINIA_RENDERING_DEVICES" );
        if( t == NULL ) {
            if( m_logger_callback != NULL ) {
                m_logger_callback( m_logger_data, 0, package.c_str(),
                                   "TINIA_RENDERING_DEVICES environment variable not set." );
            }
            return false;
        }
        displays = std::string( t );
    }
    // The display string can contain a ;-separated list of rendering devices,
    // however, we currently only support one, the first one in the list.
    std::string display = displays.substr( 0, displays.find(';') );
    if( display.empty() ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Unable to determine first display in '%s'",
                               displays.c_str() );
        }
        return false;
    }
    if( debug ) {
        m_context.requestDebug();
    }
    
    // Try to setup context.
    if( !m_context.setupContext( display ) ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Failed to create OpenGL context on '%s'.",
                               display.c_str() );
        }
        return false;
    }
    if( !m_context.bindContext() ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Failed to bind OpenGL context on '%s'.",
                               display.c_str() );
        }
        return false;
    }
    if( m_logger_callback != NULL ) {
        m_logger_callback( m_logger_data, 2, package.c_str(),
                           "Created OpenGL context on '%s'", display.c_str() );
    }
    
    // --- OpenGL context created, init glew etc. ------------------------------
    glewInit();

    if( m_logger_callback != NULL ) {
        GLint major, minor;
        glGetIntegerv( GL_MAJOR_VERSION, &major );
        glGetIntegerv( GL_MINOR_VERSION, &minor );
        
        m_logger_callback( m_logger_data, 2, package.c_str(),
                           "OpenGL %d.%d (%s, %s, %s).",
                           major, minor,
                           glGetString( GL_RENDERER ),
                           glGetString( GL_VERSION ),
                           glGetString( GL_VENDOR ) );
    }
    glGetIntegerv( GL_MAX_INTEGER_SAMPLES, &m_max_samples );

#ifdef GLEW_khr_debug // Make sure we have a new enough glew version
    if( debug ) {
        if( glewIsSupported( "GL_KHR_debug" ) ) {
            GLDebugLogWrapperData* data = new GLDebugLogWrapperData;
            data->m_logger_callback = m_logger_callback;
            data->m_logger_data = m_logger_data;
            
            glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
            glDebugMessageCallback( GLDebugLogWrapper, data );
            glDebugMessageControl( GL_DONT_CARE,
                                   GL_DONT_CARE,
                                   GL_DEBUG_SEVERITY_NOTIFICATION,
                                   0, NULL, GL_TRUE );
            if( m_logger_callback != NULL ) {
                m_logger_callback( m_logger_data, 2, package.c_str(),
                                   "Enabled OpenGL debugging." );
            }
        }
        else {
            if( m_logger_callback != NULL ) {
                m_logger_callback( m_logger_data, 2, package.c_str(),
                                   "GL_KHR_debug not supported." );
            }
        }
    }
#endif
    
    bool ipcRetVal = IPCJobController::init();
    return (ipcRetVal && m_openGLJob->initGL());

}

bool
IPCGLJobController::onGetRenderlist( size_t&             result_size,
                                   char*               result_buffer,
                                   const size_t        result_buffer_size,
                                   const std::string&  session,
                                   const std::string&  key,
                                   const std::string&  timestamp )
{
    // FIXME: Send this as an uint all the way through.
    unsigned int client_revision = 0;
    try {
        client_revision = boost::lexical_cast<unsigned int>( timestamp );
    }
    catch( boost::bad_lexical_cast& e ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Failed to parse timestamp '%s'.",
                               timestamp.c_str() );
        }
    }
    if( m_openGLJob == NULL ) {
        return false;
    }
    const renderlist::DataBase* db = m_openGLJob->getRenderList( session, key );
    if( db == NULL ) {
        result_size = 0;
        return true;
    }

    std::string list = renderlist::getUpdateXML( db,
                                                    renderlist::ENCODING_JSON,
                                                    client_revision );
    if( list.length()+1 < result_buffer_size ) {
        strcpy(result_buffer, list.c_str());
        result_size = list.size();
        return true;
    }
    else {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Insufficient buffer size for render list.",
                               timestamp.c_str() );
        }
        return false;
    }
}


void
IPCGLJobController::dumpEnvironmentList()
{
    if( m_logger_callback != NULL ) {
        std::stringstream o;
        
        for( auto it = m_environments.begin(); it!=m_environments.end(); ++it ) {
            o << "["
              << (*it)->m_width
              << "x"
              << (*it)->m_height
              << "x"
              << (*it)->m_samples
              << "] ";
        }
        m_logger_callback( m_logger_data, 2, package.c_str(),
                           "environments: %s",
                           o.str().c_str() );
    }
}


IPCGLJobController::RenderEnvironment*
IPCGLJobController::getRenderEnvironment( GLsizei width, GLsizei height, GLsizei samples )
{
    // --- bump width and height up to the nearest power-of-two ----------------
    GLsizei w2 = width - 1;
    w2 = w2 | (w2>>1);
    w2 = w2 | (w2>>2);
    w2 = w2 | (w2>>4);
    w2 = w2 | (w2>>8);
    w2 = w2 | (w2>>16);
    w2 = w2 + 1;
    GLsizei h2 = height - 1;
    h2 = h2 | (h2>>1);
    h2 = h2 | (h2>>2);
    h2 = h2 | (h2>>4);
    h2 = h2 | (h2>>8);
    h2 = h2 | (h2>>16);
    h2 = h2 + 1;

    // --- Check if we already have a suitable environment ---------------------    
    for( auto it = m_environments.begin(); it!=m_environments.end(); ++it ) {
        if( ((*it)->m_width == w2 ) && ((*it)->m_height == h2) && ((*it)->m_samples == samples ) ) {
            // move to front
            if( it != m_environments.begin() ) {
                m_environments.splice( m_environments.begin(),
                                       m_environments,
                                       it,
                                       std::next( it ) );
                dumpEnvironmentList();
            }
            
            return *it;
        }
    }
    
    // --- Delete the least recently used environments -------------------------
    while( m_environments.size() > 10 ) {
        glDeleteFramebuffers( 1, &m_environments.back()->m_fbo );
        glDeleteRenderbuffers( 1, &m_environments.back()->m_renderbuffer_rgba );
        glDeleteRenderbuffers( 1, &m_environments.back()->m_renderbuffer_depth );
        delete m_environments.back();
        m_environments.pop_back();
    }

    RenderEnvironment* e = new RenderEnvironment;
    
    glGenFramebuffers( 1, &e->m_fbo );
    glGenRenderbuffers( 1, &e->m_renderbuffer_rgba );
    glGenRenderbuffers( 1, &e->m_renderbuffer_depth );

    if( samples > 1 ) {
        glBindRenderbuffer( GL_RENDERBUFFER, e->m_renderbuffer_rgba );
        glRenderbufferStorageMultisample( GL_RENDERBUFFER, samples, GL_RGBA, w2, h2 );
        glBindRenderbuffer( GL_RENDERBUFFER, e->m_renderbuffer_depth );
        glRenderbufferStorageMultisample( GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT, w2, h2 );
    }
    else {
        glBindRenderbuffer( GL_RENDERBUFFER, e->m_renderbuffer_rgba );
        glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA, w2, h2 );
        glBindRenderbuffer( GL_RENDERBUFFER, e->m_renderbuffer_depth );
        glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w2, h2 );
    }
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );

    glBindFramebuffer( GL_FRAMEBUFFER, e->m_fbo );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, e->m_renderbuffer_rgba );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, e->m_renderbuffer_depth );

    if(!checkFramebufferCompleteness() ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Failed to create FBO [%dx%dx%d]",
                               w2, h2, samples );
        }
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glDeleteFramebuffers( 1, &e->m_fbo );
        glDeleteRenderbuffers( 1, &e->m_renderbuffer_rgba );
        glDeleteRenderbuffers( 1, &e->m_renderbuffer_depth );
        delete e;
        return NULL;
    }
    else {
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        e->m_width = w2;
        e->m_height = h2;
        e->m_samples = samples;
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 2, package.c_str(),
                               "Created FBO [%dx%dx%d]",
                               w2, h2, samples );
        }
        m_environments.push_front( e );
        dumpEnvironmentList();
        return e;
    }
    
}


bool
IPCGLJobController::onGetSnapshot( char*               buffer,
                                   TrellPixelFormat    pixel_format,
                                   const size_t        width,
                                   const size_t        height,
                                   const size_t        depth_width,
                                   const size_t        depth_height,
                                   const bool          depth16,
                                   const bool          dump_images,
                                   const std::string&  session,
                                   const std::string&  key )
{
    // bind context
    if( !m_context.bindContext() ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Failed to bind OpenGL context." );
        }
        return false;
    }
    // check if we have a key (and we should check if this corrensponds to a viewer)
    if( key.empty() ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "No key given." );
        }
        return false;
    }
    // sane size
    if( width < 1 || height < 1 ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Size must be at least 1x1 pixels." );
        }
        return false;
    }
    
    GLsizei samples = std::min( std::max( 0,
                                          (m_max_samples*m_quality+127)/255),
                                m_max_samples );
    

    // --- get render targets --------------------------------------------------    
    RenderEnvironment* env_render = getRenderEnvironment( width, height,
                                                          samples );
    if( env_render == NULL ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Failed to get rendering environment" );
        }
        return false;
    }
    RenderEnvironment* env_copy = env_render;
    if( env_render->m_samples > 1 ) {
        env_copy = getRenderEnvironment( width, height, 1 );
        if( env_copy == NULL ) {
            if( m_logger_callback != NULL ) {
                m_logger_callback( m_logger_data, 0, package.c_str(),
                                   "Failed to get de-msaa environment" );
            }
            return false;
        }
    }

    // --- render --------------------------------------------------------------
    glBindFramebuffer( GL_FRAMEBUFFER, env_render->m_fbo );
    glViewport( 0, 0, width, height );

    if( !checkForGLError() ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Got OpenGL error before Job's rendering code." );
        }
        return false;
    }

    if( env_render->m_samples > 1 ) {
        glEnable( GL_MULTISAMPLE );
    }
    bool res = m_openGLJob->renderFrame( session, key, env_render->m_fbo, width, height );
    if( env_render->m_samples > 1 ) {
        glDisable( GL_MULTISAMPLE );
    }
    
    if( res == false ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Job's rendering code failed" );
        }
    }
    if( !checkForGLError() ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Got OpenGL error in Job's rendering code." );
        }
        return false;
    }

    // --- if multisample, blit to non-multisample before reading --------------
    if( env_render != env_copy ) {
        glBindFramebuffer( GL_DRAW_FRAMEBUFFER, env_copy->m_fbo );
        glBindFramebuffer( GL_READ_FRAMEBUFFER, env_render->m_fbo );
        glBlitFramebuffer( 0, 0, width, height,
                           0, 0, width, height,
                           GL_COLOR_BUFFER_BIT,
                           GL_NEAREST );
    }
    
    // --- read pixels ---------------------------------------------------------
    glBindFramebuffer( GL_FRAMEBUFFER, env_copy->m_fbo );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    switch( pixel_format ) {
    case TRELL_PIXEL_FORMAT_RGB_JPG_VERSION: // @@@
    case TRELL_PIXEL_FORMAT_RGB:
        glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer );
        break;
    case TRELL_PIXEL_FORMAT_RGB_CUSTOM_DEPTH:
    {
        unsigned char *buffer_pos = (unsigned char *)buffer;
        glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer_pos );


        const size_t fb_width      = 1080;
        const size_t fb_height     = 1920;
        const size_t kinect_width  = 512;
        const size_t kinect_height = 424;


        static bool firsttime = true;


        // 160805: Reading binary data from file, produced by writing out matlab scripts, see notes_tinia.txt.
        static std::vector<unsigned char> fb_img( fb_width*fb_height*3, 0 );
        static std::vector<float> depth_img( kinect_width*kinect_height, 1.0f );
        static float min_depth_read_from_file = FLT_MAX;
        static float max_depth_read_from_file = FLT_MIN;
        if (firsttime) {
            std::string fname( "/home/jnygaard/SVIP_Kinect_PCL/fb.bin" );
            FILE *f=fopen( fname.c_str(), "r" );
            if (f==NULL) {
                std::cout << "Couldn't open " << fname << " for reading." << std::endl;
            }
            size_t bytes_read = fread( &fb_img[0], 1, fb_img.size(), f );
            if ( bytes_read != fb_img.size() ) {
                std::cout << "Huh? Read " << bytes_read << " bytes for fb, expected " << fb_img.size() << std::endl;
            }
            fclose(f);

            fname ="/home/jnygaard/SVIP_Kinect_PCL/db.bin";
            f=fopen( fname.c_str(), "r" );
            if (f==NULL) {
                std::cout << "Couldn't open " << fname << " for reading." << std::endl;
            }
            bytes_read = sizeof(float) * fread( &depth_img[0], sizeof(float), depth_img.size(), f );
            if ( bytes_read != sizeof(float)*depth_img.size() ) {
                std::cout << "Huh? Read " << bytes_read << " bytes, expected " << sizeof(float)*depth_img.size() << std::endl;
            }
            fclose(f);
            for (size_t i=0; i<depth_img.size(); i++) {
                min_depth_read_from_file = std::min( min_depth_read_from_file, depth_img[i] );
                max_depth_read_from_file = std::max( max_depth_read_from_file, depth_img[i] );
            }
            std::cout << "min and max depths from binary depth file: " << min_depth_read_from_file << " " << max_depth_read_from_file << std::endl;
            std::cout << "fb width and height: " << fb_width << " " << fb_height << std::endl;
            std::cout << "dst buffer width and height: " << width << " " << height << std::endl;
            std::cout << "depth buffer width and height: " << kinect_width << " " << kinect_height << std::endl;

            firsttime = false;
        }


        // 160805: Filling the frame buffer, overwriting readpixels-results. Re-scaling without interpolation, for the time being.
        for (size_t dst_i=0; dst_i<height; dst_i++) {
            size_t src_i = size_t( floor( dst_i/double(height)*fb_height + 0.5 ) );
            for (size_t dst_j=0; dst_j<width; dst_j++) {
                size_t src_j = size_t( floor( dst_j/double(width)*fb_width + 0.5 ) );
                for (size_t k=0; k<3; k++) {
                    buffer_pos[ (dst_i*width + dst_j)*3 + k ] = fb_img[ ((fb_height-1-src_i)*fb_width + src_j) + k*fb_width*fb_height ];
                }
            }
        }

#if 0
        // Filling with solid red.
        for (size_t i=0; i<width*height; i++) {
            buffer_pos[ 3*i + 0 ] = 255;
            buffer_pos[ 3*i + 1 ] = 0;
            buffer_pos[ 3*i + 2 ] = 0;
        }
#endif


#if 0
        // 160804:
        std::cout << "Dumping frame buffer contents (after readpixels and/or image painting), lower left corner values:\n";
        for (int i=0; i<10; i++) {
            std::cout << i << ": \t";
            for (int j=0; j<10; j++) {
                std::cout << "(" << int(buffer_pos[ (width*i+j)*3 ]) << ", " << int(buffer_pos[ (width*i+j)*3 + 1 ]) << ", " << int(buffer_pos[ (width*i+j)*3 + 2 ]) << ") ";
            }
            std::cout << "\n";
        }
        std::cout << std::endl;
#endif


        buffer_pos += 4*((width*height*3 + 3)/4); // As long as GL_PACK_ALIGNMENT is set to 1 above, this should be ok. (I.e., no padding for single scan lines.)
        // NB! We read four bytes per pixel, then convert to three, meaning that the buffer must be large enough for four!!!
        // (This should be taken care of further up the stack. The caller of this method is IPCJobController::handle().)
        glReadPixels( 0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, buffer_pos );


#if 0
        // 160804:
        std::cout << "dumping depth buffer, lower left corner values, 1 seems to be \"unset\" (and infinity?), range probably [0, 1], with 0 at near plane?!\n";
        for (int i=0; i<10; i++) {
            std::cout << i << ": \t";
            for (int j=0; j<10; j++) {
                std::cout << ((float *)buffer_pos)[width*i+j] << " ";
            }
            std::cout << "\n";
        }
        std::cout << std::endl;
#endif


        // 160804: Filling the depth buffer, overwriting readpixels-results. By calling this routine with exactly the same depth_width and
        //         depth_height as the Kinect data contains (how do we do it?) we would avoid up/down-sampling...
        {
            float       * const dst_buf = (float *)buffer_pos;
            const float * const src_buf = &depth_img[0];

            // Trying to fill the whole buffer with mean of near and far plane. Seems more or less like what we expect.
//            for (size_t i=0; i<height*width; i++) {
//                dst_buf[i] = 0.5f;
//            }

            // Far plane, will not produce splats. (Do we test on the value explicitly, or is it just outside of the frustum?)
//            for (size_t i=0; i<height*width; i++) {
//                dst_buf[i] = 1.0f;
//            }

            for (size_t dst_i=0; dst_i<height; dst_i++) {
                size_t src_i = size_t( floor( dst_i/double(width)*kinect_width + 0.5 ) );
                for (size_t dst_j=0; dst_j<width; dst_j++) {
                    size_t src_j = size_t( floor( dst_j/double(height)*kinect_height + 0.5 ) );
                    dst_buf[ dst_i*width + dst_j ] =
                            ( src_buf[ (kinect_width-1-src_i)*kinect_height + src_j ] - min_depth_read_from_file ) /
                            ( max_depth_read_from_file - min_depth_read_from_file ); //  * 0.99999;
                }
            }
        }


        if( m_logger_callback != NULL ) { // (This goes to /tmp/job-id.stderr)
            m_logger_callback( m_logger_data, 0, package.c_str(), "Current canvas and depth buffer size: %d %d and %d %d", width, height, depth_width, depth_height );
        }

        if ( (depth_width!=width) || (depth_height!=height) ) {
            // New path, downsampling, without bi-linear interpolation
            // 160804: Hmm. Can we be sure depth size is always smaller or equal to image size?!
            float *tmp_buffer = new float[depth_width*depth_height];
            const float * const m_buf = (float *)buffer_pos;
            for (size_t i=0; i<depth_height; i++) {
                size_t ii = size_t( floor( (i*height)/double(depth_height) + 0.5 ) );
                for (size_t j=0; j<depth_width; j++) {
                    size_t jj = size_t( floor( (j*width)/double(depth_width) + 0.5 ) );
                    tmp_buffer[ i*depth_width + j ] = m_buf[ ii*width + jj ];
                }
            }
            if (depth16) {
                // Depth encoded as 24 bit fixed point values, least significant bits set to 0
                for (size_t i=0; i<depth_width*depth_height; i++) {
                    float value = tmp_buffer[i];
                    (buffer_pos)[3*i+0] = (unsigned char)( floor(value*255.0) );
                    value = 255.0*value - floor(value*255.0);
                    (buffer_pos)[3*i+1] = (unsigned char)( floor(value*255.0) );
                    (buffer_pos)[3*i+2] = 0;
                }
            } else {
                // Depth encoded as 24 bit fixed point values.
                for (size_t i=0; i<depth_width*depth_height; i++) {
                    float value = tmp_buffer[i];
                    for (size_t j=0; j<3; j++) {
                        (buffer_pos)[3*i+j] = (unsigned char)( floor(value*255.0) );
                        value = 255.0*value - floor(value*255.0);
                    }
                }
            }
            delete tmp_buffer;
        } else {
            // (Old path for non-reduced depth resolution)
            if (depth16) {
                // Depth encoded as 24 bit fixed point values, least significant bits set to 0
                for (size_t i=0; i<width*height; i++) {
                    float value = (float)( ((GLfloat *)buffer_pos)[i] );
                    (buffer_pos)[3*i+0] = (unsigned char)( floor(value*255.0) );
                    value = 255.0*value - floor(value*255.0);
                    (buffer_pos)[3*i+1] = (unsigned char)( floor(value*255.0) );
                    (buffer_pos)[3*i+2] = 0;
                }
            } else {
                // Depth encoded as 24 bit fixed point values.
                for (size_t i=0; i<width*height; i++) {
                    float value = (float)( ((GLfloat *)buffer_pos)[i] );
                    for (size_t j=0; j<3; j++) {
                        (buffer_pos)[3*i+j] = (unsigned char)( floor(value*255.0) );
                        value = 255.0*value - floor(value*255.0);
                    }
                }
            }
            if (dump_images) {
                static int cntr=0;
                {
                    char fname[1000];
                    sprintf(fname, "/tmp/trell_rgb_%05d.ppm", cntr);
                    FILE *fp = fopen(fname, "w");
                    fprintf(fp, "P6\n%lu\n%lu\n255\n", width, height);
                    fwrite(buffer_pos - 4*((width*height*3 + 3)/4), 1, 3*width*height, fp);
                    fclose(fp);
                }
                {
                    char fname[1000];
                    sprintf(fname, "/tmp/trell_depth_%05d.ppm", cntr);
                    FILE *fp = fopen(fname, "w");
                    fprintf(fp, "P6\n%lu\n%lu\n255\n", width, height);
                    fwrite(buffer_pos, 1, 3*width*height, fp);
                    fclose(fp);
                }
                cntr++;
            }
        }
       break;
    }
    default:
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Unsupported pixel format." );
        }
        return false;
    }

    return true;
}

void
IPCGLJobController::cleanup()
{
    IPCJobController::cleanup();
}



bool
IPCGLJobController::checkFramebufferCompleteness() const
{
    GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if( status == GL_FRAMEBUFFER_COMPLETE ) {
        return true;
    }
    if( m_logger_callback != NULL ) {
        const char* error = NULL;
        switch( status ) {
        case GL_FRAMEBUFFER_UNDEFINED:
            error = "GL_FRAMEBUFFER_UNDEFINED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            error = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            error = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            error = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            error = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            error = "GL_FRAMEBUFFER_UNSUPPORTED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            error = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            error = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
            break;
        default:
            break;
        }
        if( error != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Incomplete framebuffer: %s.", error );
        }
        else {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Incomplete framebuffer: %x", status );
        }
    }
    return false;
}

bool
IPCGLJobController::checkForGLError() const
{
    GLenum status = glGetError();
    if( status == GL_NO_ERROR ) {
        return true;
    }
    while( status != GL_NO_ERROR ) {
        if( m_logger_callback != NULL ) {
            const char* error = NULL;
            switch( status ) {
            case GL_INVALID_ENUM:
                error = "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "GL_INVALID_OPERATION";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error = "GL_OUT_OF_MEMORY";
                break;
            default:
                break;
            }
            if( error != NULL ) {
                m_logger_callback( m_logger_data, 0, package.c_str(),
                                   "OpenGL error: %s.", error );
                
            }
            else {
                m_logger_callback( m_logger_data, 0, package.c_str(),
                                   "OpenGL error: %x", status );
            }
            status = glGetError();
        }
    }
    return false;
}

} // of namespace trell
} // of namespace tinia

