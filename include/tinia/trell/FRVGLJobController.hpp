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
#include <GL/glew.h>
#include <unordered_map>


#include "tinia/jobcontroller/Controller.hpp"
#include "tinia/jobcontroller/OpenGLJob.hpp"


namespace tinia {
namespace trell {

class FRVGLJobController : public jobcontroller::Controller
{

public:

    explicit FRVGLJobController( bool is_master = false );
    void setJob( jobcontroller::Job *job);

   ////from Job.hpp
   //bool init();
   //void cleanup();
   //bool periodic();
   //void quit();

    //from Controller.hpp
    int run(int argc, char** argv);
    void finish();
    void fail();
    void addScript( const std::string& script) {}

    virtual
    bool
    init( );


    char* render( float* modelView = nullptr, float* projection = nullptr );


protected:
    /** \copydoc MessageBox::init */
    


    /** \copydoc MessageBox::cleanup */
    virtual
    void
    cleanup();



bool
FRVGLJobController::onGetSnapshot( char*               buffer,
                                   const size_t        width,
                                   const size_t        height,
                                   const std::string&  session,
                                   const std::string&  key );

private:

    jobcontroller::OpenGLJob*                           m_openGLJob;
    int                                                 m_quality;    
    GLuint                                              m_fbo;
    GLuint                                              m_renderbuffer_rgba;
    GLuint                                              m_renderbuffer_depth;
    GLsizei                                             m_width;
    GLsizei                                             m_height;
    GLsizei                                             m_samples;
    GLsizei                                             m_max_samples;

    int                                                 m_argc;
    char**                                               m_argv;
    
    bool
    checkFramebufferCompleteness() const;

    bool
    checkForGLError() const;



};


} // of namespace trell
} // of namespace tinia
