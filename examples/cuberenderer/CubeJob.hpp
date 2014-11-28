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
#include <tinia/renderlist/DataBase.hpp>
#include "tinia/jobcontroller/OpenGLJob.hpp"
#include "tinia/model/StateListener.hpp"
namespace tinia {
namespace example {
class CubeJob : public tinia::jobcontroller::OpenGLJob, public tinia::model::StateListener
{
public:
    CubeJob();
    ~CubeJob();
    void stateElementModified(tinia::model::StateElement *stateElement);
    bool init();
    bool renderFrame(const std::string &session, const std::string &key,
                     unsigned int fbo, const size_t width, const size_t height);

    /** Database for renderlist.
      *
      * Desktop jobs doesn't usually provide render lists (as they are normally
      * not used by qtcontroller). However, for debug purposes it is possible to
      * tell qtcontroller to use render lists, so we have included code to
      * demonstrate this.
      */
    const tinia::renderlist::DataBase*
    getRenderList(const std::string &session, const std::string &key);

    float rotate( float r );

private:
    tinia::renderlist::DataBase     m_renderlist_db;
    GLuint                      m_gpgpu_quad_vertex_array;
    GLuint                      m_gpgpu_quad_buffer;
    float                       m_rotation;
};
}
}
