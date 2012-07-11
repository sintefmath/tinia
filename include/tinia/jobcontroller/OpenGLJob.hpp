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

#include "tinia/jobcontroller/Job.hpp"
#include <tinia/renderlist/RenderList.hpp>

class QResizeEvent;
class QKeyEvent;
class QMouseEvent;

namespace tinia {
namespace jobcontroller {

/** OpenGLJob is responsible for rendering OpenGL.
 * The user will typically override renderFrame and getRenderList to
 * specify the rendering process.
 */
class OpenGLJob : public Job
{
public:
   OpenGLJob();

   /** When this method is called, all OpenGL-methods should be safe to call (glewInit etc.)
   */
   virtual bool initGL() { return true; }

   /** Render the view of a given key using the current GL context and bound FBO.
    * Reimplement this to use your own renderloop.
         *
         * \param session  Identity of the client where the request originated.
         * \param key      The model-key of the view that should be rendered.
         * \param fbo      The fbo being rendered to.
         * \param width    The current width of the FBO.
         * \param height   The current height of the FBO.
         * \returns True if the FBO contains valid data.
         * \note Invoked by IPC thread.*/
   virtual
   bool
   renderFrame(

           const std::string&  session,
           const std::string&  key,
           unsigned int fbo,
           const size_t        width,
           const size_t        height )=0;



    /** Let the controller access the render list for a particular session and key.
      *
      * Return null if the job doesn't want to provide a render list.
      * \note Invoked by IPC thread.
      */
    virtual
    const renderlist::DataBase*
    getRenderList( const std::string& session,
                   const std::string& key ) { return NULL; }



protected:
};
}
}


