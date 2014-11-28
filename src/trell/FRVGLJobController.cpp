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
#include "tinia/trell/FRVGLJobController.hpp"
#include "tinia/trell/GLDebugMessages.hpp"
#include <GL/freeglut.h>


//not the way it should be, but for proof of concept and bypassing exposedModel, here we go
#include "../../examples/cuberenderer/CubeJob.hpp"

namespace {

static const std::string package = "FRVGLJobController";
} // of anonymous namespace


namespace tinia {
namespace trell {


FRVGLJobController::FRVGLJobController(bool is_master)
    : m_openGLJob( NULL )
{
    int argc = 1;
    char* argv = "FRVGLJobController";
}

void FRVGLJobController::setJob( jobcontroller::Job* job )
{
    //no need to have it saved as anything else afaik
    m_openGLJob = static_cast<jobcontroller::OpenGLJob*>(job);
}

bool
FRVGLJobController::init()
{

    
    // --- set up OpenGL context ------------------------------------------------

    //set up Freeglut window here
    //is init called after run? If so I could use the actual values, but not really worth wile..
    
    glutInit(&m_argc, m_argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize (1000, 1000);
    glutInitWindowPosition (1000, 100);
    glutInitContextFlags( GLUT_DEBUG );
    glutCreateWindow ("FRViewJobDebugWindow");    
    
    // --- OpenGL context created, init glew etc. ------------------------------
    glewInit();

    GLDebugMessages::setupGLDebugMessages();

    
    GLint major, minor;
    glGetIntegerv( GL_MAJOR_VERSION, &major );
    glGetIntegerv( GL_MINOR_VERSION, &minor );
    glGetIntegerv( GL_MAX_INTEGER_SAMPLES, &m_max_samples );
    std::stringstream ss;
    ss << "OpenGL " << major <<"."<<minor<<
        " " << glGetString( GL_RENDERER ) <<
        " " << glGetString( GL_VERSION ) <<
        " " << glGetString( GL_VENDOR ); 
    glDebugMessageInsert( GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 112, GL_DEBUG_SEVERITY_NOTIFICATION,
                          -1, ss.str().c_str() );


    //set up the FBO that we will be rendering to
    m_fbo = 0; //direct to screen right now please.


   
    
    //initialise OpenGL job
    return m_openGLJob->init() && m_openGLJob->initGL();

}


bool
FRVGLJobController::onGetSnapshot( char*               buffer,
                                   const size_t        width,
                                   const size_t        height,
                                   const std::string&  session,
                                   const std::string&  key )
{
   
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
    
    

    // --- render --------------------------------------------------------------
    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
    glViewport( 0, 0, width, height );

    if( !checkForGLError() ) {
        if( m_logger_callback != NULL ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Got OpenGL error before Job's rendering code." );
        }
        return false;
    }

    
    //    glEnable( GL_MULTISAMPLE );
    
    bool res = m_openGLJob->renderFrame( session, key,m_fbo, width, height );
    //    glDisable( GL_MULTISAMPLE );
    
    
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
    
      //  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_fbo );
      //  glBindFramebuffer( GL_READ_FRAMEBUFFER, m_fbo );
      //  glBlitFramebuffer( 0, 0, width, height,
      //                     0, 0, width, height,
      //                     GL_COLOR_BUFFER_BIT,
      //                     GL_NEAREST );
    
    
    // --- read pixels ---------------------------------------------------------
    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );

        glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer );

   // case TRELL_PIXEL_FORMAT_RGB_CUSTOM_DEPTH:
   // {
   //     unsigned char *buffer_pos = (unsigned char *)buffer;
   //     glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer_pos );
   //     buffer_pos += 4*((width*height*3 + 3)/4); // As long as GL_PACK_ALIGNMENT is set to 1 above, this should be ok. (I.e., no padding for single scan lines.)
   //     // NB! We read four bytes per pixel, then convert to three, meaning that the buffer must be large enough for four!!!
   //     glReadPixels( 0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, buffer_pos );
   //     // Depth encoded as 24 bit fixed point values.
   //     for (size_t i=0; i<width*height; i++) {
   //         float value = (float)( ((GLfloat *)buffer_pos)[i] );
   //         for (size_t j=0; j<3; j++) {
   //             (buffer_pos)[3*i+j] = (unsigned char)( floor(value*255.0) );
   //             value = 255.0*value - floor(value*255.0);
   //         }
   //     }
   //
   //    break;
   // }
   
        //do jpeg encoding and transmit the thing back here.
        glutSwapBuffers();

    return true;
}

void
FRVGLJobController::cleanup()
{
    //nuke window and such
    //or just don't do anything..
}



bool
FRVGLJobController::checkFramebufferCompleteness() const
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
FRVGLJobController::checkForGLError() const
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

void FRVGLJobController::finish()
{
    m_openGLJob->quit();
}

void FRVGLJobController::fail()
{
    std::cerr << "will never ever be called, as this prototype does not support this call" << std::endl;
}

int FRVGLJobController::run( int argc, char** argv )
{
    m_argc = argc;
    m_argv = argv;
    //setup polling of websocket messages
    //ie update/idle function
    if( !init() ){
        glDebugMessageInsert( GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 113, GL_DEBUG_SEVERITY_HIGH, -1, "Could not initialise OpenGLJob, terminating");
        exit(113);
    }
    //call glutMainloop.



    m_openGLJob->renderFrame( "blah", "viewer", 0, 512, 512 );
    glutSwapBuffers();
    
    static float rotation = 0.001;
    float prev = 0;
    tinia::example::CubeJob* cj = (tinia::example::CubeJob*) m_openGLJob;
    for (int i = 0; i < 150; i++ )
    {
        prev = cj->rotate( rotation * i );

        m_openGLJob->renderFrame( "blah", "viewer", 0, 512, 512 );
        glutSwapBuffers();
    }

    return 1;
}



} // of namespace trell
} // of namespace tinia

