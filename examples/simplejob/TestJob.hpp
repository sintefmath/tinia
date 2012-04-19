#pragma once
#include <GL/glew.h>
#include <librenderlist/DataBase.hpp>
#include "jobobserver/OpenGLJob.hpp"
#include "policylib/StateListener.hpp"
class TestJob : public jobobserver::OpenGLJob, public policylib::StateListener
{
public:
    TestJob();
    ~TestJob();
    void stateElementModified(policylib::StateElement *stateElement);
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
    const librenderlist::DataBase*
    getRenderList(const std::string &session, const std::string &key);

private:
    librenderlist::DataBase     m_renderlist_db;
    GLuint                      m_gpgpu_quad_vertex_array;
    GLuint                      m_gpgpu_quad_buffer;
};
