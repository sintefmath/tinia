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

class Triangle {
public:
    Triangle();
    ~Triangle();
    bool initGL();

    bool render(float *modelview, float *projection);

private:
    GLuint m_buffer;
    GLuint m_vao;
};


Triangle::Triangle()
    : m_buffer(NULL), m_vao(NULL)
{
}

Triangle::~Triangle()
{
    if(m_buffer !=  NULL) {
        glDeleteBuffers(1, &m_buffer);
    }
    if(m_vao != NULL) {
        glDeleteVertexArrays(1, &m_vao);
    }
}

bool Triangle::initGL()
{
    float vertices[] = {0, 0, 0.5,
                        1, 0, 0.5,
                        1, 1, 0.5};

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glGenBuffers(1, &m_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
    glBufferData( GL_ARRAY_BUFFER,  sizeof(vertices), vertices, GL_STATIC_DRAW);
    //glVertexPointer(3, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

bool Triangle::render(float *modelview, float *projection)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
    glVertexPointer(4, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES, 0, 1);
}
