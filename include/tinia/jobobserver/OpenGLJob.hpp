#pragma once

#include "tinia/jobobserver/Job.hpp"
#include <tinia/librenderlist/RenderList.hpp>
namespace tinia {
namespace jobobserver {
class OpenGLJob : public Job
{
public:
   OpenGLJob();
   /**
     TODO Should we include the fbo?
     */
   /** Render the view of a given key using the current GL context and bound FBO.
         *
         * \param[in] session  Identity of the client where the request originated.
         * \param[in] key      The policy-key of the view that should be rendered.
         * \param[in] fbo      The fbo being rendered to.
         * \param[in] width    The current width of the FBO.
         * \param[in] height   The current height of the FBO.
         * \returns True if the FBO contains valid data.
         * \note Invoked by IPC thread.
         */


   /**
   When this method is called, all OpenGL-methods should be safe to call (glewInit etc.)
   */
   virtual bool initGL() { return true; }
   virtual
   bool
   renderFrame(

         const std::string&  session,
                const std::string&  key,
                unsigned int fbo,
                const size_t        width,
                const size_t        height )=0;



    /** Let the observer access the render list for a particular session and key.
      *
      * Return null if the job doesn't want to provide a render list.
      * \note Invoked by IPC thread.
      */
    virtual
    const librenderlist::DataBase*
    getRenderList( const std::string& session,
                   const std::string& key ) { return NULL; }

protected:
};
}
}


