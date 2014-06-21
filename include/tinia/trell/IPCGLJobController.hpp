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
#include "tinia/jobcontroller/OpenGLJob.hpp"
#include "tinia/trell/OffscreenGL.hpp"
#include "IPCJobController.hpp"

namespace tinia {
namespace trell {

class IPCGLJobController : public IPCJobController
{
public:

    /** Set rendering quality.
     *
     * \param[in] quality  A number in the range [0..255], where 255 is the
     *                     highest quality.
     *
     * This is a kludge to let specific jobs enable MSAA, and the API and
     * behaviour WILL most likely change.
     *
     */
    void
    setQuality( int quality );
    
    IPCGLJobController( bool is_master = false );

protected:
    /** \copydoc MessageBox::init */
    virtual
    bool
    init( );


    /** \copydoc MessageBox::cleanup */
    virtual
    void
    cleanup();

    virtual
    bool
    onGetSnapshot( char*               buffer,
                   TrellPixelFormat    pixel_format,
                   const size_t        width,
                   const size_t        height,
                   const std::string&  session,
                   const std::string&  key );

    virtual
    bool
    onGetRenderlist( size_t&             result_size,
                     char*               buffer,
                     const size_t        buffer_size,
                     const std::string&  session,
                     const std::string&  key,
                     const std::string&  timestamp );



private:
    jobcontroller::OpenGLJob*                           m_openGLJob;
    impl::OffscreenGL                                   m_context;
    int                                                 m_quality;
    struct RenderEnvironment {
        GLuint                                          m_fbo;
        GLuint                                          m_renderbuffer_rgba;
        GLuint                                          m_renderbuffer_depth;
        GLsizei                                         m_width;
        GLsizei                                         m_height;
        GLsizei                                         m_samples;
    };

    std::list<RenderEnvironment*>                       m_environments;

    GLsizei                                             m_max_samples;


    std::unordered_map<std::string, RenderEnvironment>  m_render_environments;

    RenderEnvironment*
    getRenderEnvironment( GLsizei width, GLsizei height, GLsizei samples );
    
    void
    dumpEnvironmentList();
    
    bool
    checkFramebufferCompleteness() const;

    bool
    checkForGLError() const;


};


} // of namespace trell
} // of namespace tinia
