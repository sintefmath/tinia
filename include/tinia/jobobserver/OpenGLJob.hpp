#pragma once

#include "tinia/jobobserver/Job.hpp"
#include <tinia/renderlist/RenderList.hpp>
#ifdef TINIA_PASS_THROUGH
class QResizeEvent;
class QKeyEvent;
class QMouseEvent;
#endif
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
    const renderlist::DataBase*
    getRenderList( const std::string& session,
                   const std::string& key ) { return NULL; }

#ifdef TINIA_PASS_THROUGH
    /** \deprecated Do not use */
    virtual void resizeEvent(QResizeEvent *event) { }
    /** \deprecated Do not use */
    virtual void mousePressEvent(QMouseEvent *event) { }
    /** \deprecated Do not use */
    virtual void mouseMoveEvent(QMouseEvent *event) { }
    /** \deprecated Do not use */
    virtual void mouseReleaseEvent(QMouseEvent *event) { }
    /** \deprecated Do not use */
    virtual void keyPressEvent(QKeyEvent *event) { }
    /** \deprecated Do not use */
    virtual void keyReleaseEvent(QKeyEvent *) { }
    /** \deprecated Do not use */
    virtual bool passThrough() { return false; }
#endif

protected:
};
}
}


