#pragma once
#include <GL/glew.h>
#include <tinia/librenderlist/DataBase.hpp>
#include "tinia/jobobserver/OpenGLJob.hpp"
#include "tinia/policy/StateListener.hpp"
class TestJob : public tinia::jobobserver::OpenGLJob, public tinia::policy::StateListener
{
public:
    TestJob();
    ~TestJob();
    void stateElementModified(tinia::policy::StateElement *stateElement);
    bool init();
    bool renderFrame(const std::string &session, const std::string &key,
                     unsigned int fbo, const size_t width, const size_t height);

    /** Database for renderlist.
      *
      * Desktop jobs doesn't usually provide render lists (as they are normally
      * not used by qtobserver). However, for debug purposes it is possible to
      * tell qtobserver to use render lists, so we have included code to
      * demonstrate this.
      */
    const tinia::librenderlist::DataBase*
    getRenderList(const std::string &session, const std::string &key);

private:
    tinia::librenderlist::DataBase     m_renderlist_db;
    GLuint                      m_gpgpu_quad_vertex_array;
    GLuint                      m_gpgpu_quad_buffer;
};
