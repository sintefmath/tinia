/* -*- mode: C++; tab-width:4; c-basic-offset: 4; indent-tabs-mode:nil -*- */
/************************************************************************
 *
 *  File: RenderCoordAxis.hpp
 *
 *  Created: 2009-08-14
 *
 *  Version:
 *
 *  Authors: Erik Bjønnes <erik.bjonnes@sintef.no>
 *
 *  This file is part of the siut library.
 *  Copyright (C) 2009 by SINTEF.  All rights reserved.
 *
 *  SINTEF, Pb 124 Blindern, N-0314 Oslo, Norway
 *  http://www.sintef.no
 *********************************************************************/
#pragma once


#include <GL/glew.h>

#include <string>
#include <sstream>

#include <stdexcept>

#include "siut2/dsrv/DSRViewer.hpp"
#include "siut2/gl_utils/ContextResourceCache.hpp"
#include "siut2/gl_utils/GLSLtools.hpp"

/** Macro used for specifying offset into vbo*/
#define BUFF_OFFSET(i) ((char*)NULL+(i))


namespace siut2 {
  namespace gl_utils {

    /** Class that renders a little cross of the axis.
     * 
     * \Usage Set where you want the cross with glViewport. 
     *
     */
    class RenderCoordAxisResource
      : public ContextResource
    {

    /** Sets up and renders an axis cross.
     * \param rc ContextResourceCache in use by the program
     * \param forward_compatible Default = true, creates an openGL 3.0 forward compatible cross.
     */
      friend void renderCoordAxis(const float *mvp, ContextResourceCache *rc, bool forward_compatible = true);

    public:
      /** Default constructor, forward compatible. */
      RenderCoordAxisResource()	
      {
      }

      /** Construct, allows you to specify if you want to be forward compatible (true) or not (false). 
       * \param forward_compatible when true will create a 3.0 forward compatible cross, if false it will use OpenGL 2 shaders.
       */
      RenderCoordAxisResource(bool forward_compatible)
	:RCAR_vbo_ (0),
	 RCAR_shader_prog_(0),
	 RCAR_pos_loc_(0),
	 RCAR_col_loc_(0),
	 RCAR_mvp_loc_(0),
	 RCAR_num_verts_(0)
      {
	setUpShader(forward_compatible);
	setUpCoordSystem();
      }

 protected:
      GLuint RCAR_vbo_;
      GLuint RCAR_shader_prog_;
      GLuint RCAR_pos_loc_;
      GLuint RCAR_col_loc_;
      GLuint RCAR_mvp_loc_;
      GLint RCAR_num_verts_;
	
    

      struct RCAR_Vertex
      {
	GLfloat pos[3];
	GLfloat col[3];

	RCAR_Vertex()
	{
	  ;
	}

	RCAR_Vertex(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b)
	{
	  pos[0] = x; pos[1] = y; pos[2] = z;
	  col[0] = r; col[1] = g; col[2] = b;
	}
      };      
      void setUpShader(bool fc)
      {
	GLuint RCAR_vert_prog_id;
	GLuint RCAR_frag_prog_id;
	std::string RCAR_vert_prog;
	std::string RCAR_frag_prog;
	std::stringstream temp_s;

	if(fc)
	  { //forward compatible!
	     temp_s << "#version 130" << std::endl;
	     temp_s << "uniform mat4 mvp;" << std::endl;
	     temp_s << "in vec3 in_pos;" << std::endl;
	     temp_s << "in vec3 in_col;"  << std::endl;
	     temp_s << "out vec3 frag_col;" << std::endl;
	     temp_s << "void main(void)" << std::endl;
	     temp_s << "{" << std::endl;
	     temp_s << "gl_Position = (mvp * vec4(in_pos, 1.0));" <<std::endl;
	     temp_s << "frag_col = in_col;" << std::endl;
	     temp_s << "}" << std::endl;
	     RCAR_vert_prog = temp_s.str();
	     temp_s.str("");
	     
	     temp_s << "#version 130" << std::endl;
	     temp_s << "in vec3 frag_col;" << std::endl;
	     temp_s << "out vec4 out_col;" << std::endl;
	     temp_s << "void main(void)" << std::endl;
	     temp_s << "{" << std::endl;
	     temp_s << "out_col = vec4(frag_col, 1.0);" << std::endl;
	     temp_s << "}" << std::endl;
	     RCAR_frag_prog = temp_s.str();
	     temp_s.str("");
	  }
	else
	  { 
	    //DEPRECATED!! DO NOT USE EXCEPT IN NEED...
	    temp_s << "uniform mat4 mvp;" << std::endl;
	    temp_s << "attribute vec3 in_pos;" << std::endl;
	    temp_s << "attribute vec3 in_col;"  << std::endl;
	    temp_s << "varying vec3 frag_col;" << std::endl;
	    temp_s << "void main(void)" << std::endl;
	    temp_s << "{" << std::endl;
	    temp_s << "gl_Position = (mvp * vec4(in_pos, 1.0));" <<std::endl;
	    temp_s << "frag_col = in_col;" << std::endl;
	    temp_s << "}" << std::endl;
	    RCAR_vert_prog = temp_s.str();
	    temp_s.str("");
	    
	    temp_s << "in vec3 frag_col;" << std::endl;
	    temp_s << "void main(void)" << std::endl;
	    temp_s << "{" << std::endl;
	    temp_s << "gl_FragColor = vec4(frag_col, 1.0);" << std::endl;
	    temp_s << "}" << std::endl;
	    RCAR_frag_prog = temp_s.str();
	    temp_s.str("");
	  }

	RCAR_vert_prog_id = gl_utils::compileShader(RCAR_vert_prog, GL_VERTEX_SHADER);
	RCAR_frag_prog_id = gl_utils::compileShader(RCAR_frag_prog, GL_FRAGMENT_SHADER);

	RCAR_shader_prog_ = glCreateProgram();
	gl_utils::printGLError(__FILE__,__LINE__);
    
	glAttachShader(RCAR_shader_prog_, RCAR_vert_prog_id);
	gl_utils::printGLError(__FILE__,__LINE__);

	glAttachShader(RCAR_shader_prog_, RCAR_frag_prog_id);
	gl_utils::printGLError(__FILE__,__LINE__);
    

	glLinkProgram(RCAR_shader_prog_);
	gl_utils::printGLError(__FILE__,__LINE__);
	
	glValidateProgram(RCAR_shader_prog_);
	gl_utils::printGLError(__FILE__,__LINE__);	
   
	glUseProgram(RCAR_shader_prog_);
	gl_utils::printGLError(__FILE__, __LINE__);

	RCAR_pos_loc_ = glGetAttribLocation(RCAR_shader_prog_, "in_pos");
	gl_utils::printGLError(__FILE__,__LINE__);
  
	RCAR_col_loc_ = glGetAttribLocation(RCAR_shader_prog_, "in_col");
	gl_utils::printGLError(__FILE__,__LINE__);
  
	RCAR_mvp_loc_ = glGetUniformLocation(RCAR_shader_prog_, "mvp");
	gl_utils::printGLError(__FILE__,__LINE__);
      }


      void setUpCoordSystem()
      {


	//set up vertices
	RCAR_num_verts_ = 36;
	std::vector<RCAR_Vertex> vert_col(RCAR_num_verts_);
	//RCAR_Vertex vert_col[RCAR_num_verts_];
	vert_col[0] = RCAR_Vertex(0.f, 0.1f, 0.f, 1.f, 0.f, 0.f);
	vert_col[1] = RCAR_Vertex(0.f, 0.f, 0.1f, 1.f, 0.f, 0.f);
	vert_col[2] = RCAR_Vertex(1.f, 0.f, 0.f, 1.f, 0.f, 0.f);
    
	vert_col[3] = RCAR_Vertex(0.f, 0.f, 0.1f, 1.f, 0.f, 0.f);
	vert_col[4] = RCAR_Vertex(0.f, -0.1f, 0.f, 1.f, 0.f, 0.f);
	vert_col[5] = RCAR_Vertex(1.f, 0.f, 0.f, 1.f, 0.f, 0.f);

	vert_col[6] = RCAR_Vertex(0.f, -0.1f, 0.f, 1.f, 0.f, 0.f);
	vert_col[7] = RCAR_Vertex(0.f, 0.f, -0.1f, 1.f, 0.f, 0.f);
	vert_col[8] = RCAR_Vertex(1.f, 0.f, 0.f, 1.f, 0.f, 0.f);

	vert_col[9] = RCAR_Vertex(0.f, 0.f, -0.1f, 1.f, 0.f, 0.f);
	vert_col[10]= RCAR_Vertex(0.f, 0.1f, 0.f, 1.f, 0.f, 0.f);
	vert_col[11]= RCAR_Vertex(1.f, 0.f, 0.f, 1.f, 0.f, 0.f);
	//y
	vert_col[12]= RCAR_Vertex(0.1f, 0.f, 0.f, 0.f, 1.f, 0.f);
	vert_col[13]= RCAR_Vertex(0.f, 0.f, 0.1f, 0.f, 1.f, 0.f);
	vert_col[14]= RCAR_Vertex(0.f, 1.f, 0.f, 0.f, 1.f, 0.f);
    
	vert_col[15]= RCAR_Vertex(0.f, 0.f, 0.1f, 0.f, 1.f, 0.f);
	vert_col[16]= RCAR_Vertex(-0.1f, 0.f, 0.f, 0.f, 1.f, 0.f);
	vert_col[17]= RCAR_Vertex(0.f, 1.f, 0.f, 0.f, 1.f, 0.f);

	vert_col[18]= RCAR_Vertex(-0.1f, 0.f, 0.f, 0.f, 1.f, 0.f);
	vert_col[19]= RCAR_Vertex(0.f, 0.f, -0.1f, 0.f, 1.f, 0.f);
	vert_col[20]= RCAR_Vertex(0.f, 1.f, 0.f, 0.f, 1.f, 0.f);

	vert_col[21]= RCAR_Vertex(0.f, 0.f, -0.1f, 0.f, 1.f, 0.f);
	vert_col[22]= RCAR_Vertex(0.1f, 0.f, 0.f, 0.f, 1.f, 0.f);
	vert_col[23]= RCAR_Vertex(0.f, 1.f, 0.f, 0.f, 1.f, 0.f);

	//z
	vert_col[24]= RCAR_Vertex(0.f, 0.1f, 0.f, 0.f, 0.f, 1.f);
	vert_col[25]= RCAR_Vertex(0.1f, 0.f, 0.f, 0.f, 0.f, 1.f);
	vert_col[26]= RCAR_Vertex(0.f, 0.f, 1.f, 0.f, 0.f, 1.f);
    
	vert_col[27]= RCAR_Vertex(0.1f, 0.f, 0.f, 0.f, 0.f, 1.f);
	vert_col[28]= RCAR_Vertex(0.f, -0.1f, 0.f, 0.f, 0.f, 1.f);
	vert_col[29]= RCAR_Vertex(0.f, 0.f, 1.f, 0.f, 0.f, 1.f);
    
	vert_col[30]= RCAR_Vertex(0.f, -0.1f, 0.f, 0.f, 0.f, 1.f);
	vert_col[31]= RCAR_Vertex(-0.1f, 0.f, 0.f, 0.f, 0.f, 1.f);
	vert_col[32]= RCAR_Vertex(0.f, 0.f, 1.f, 0.f, 0.f, 1.f);
    
	vert_col[33]= RCAR_Vertex(-0.1f, 0.f, 0.f, 0.f, 0.f, 1.f);
	vert_col[34]= RCAR_Vertex(0.f, 0.1f, 0.f, 0.f, 0.f, 1.f);
	vert_col[35]= RCAR_Vertex(0.f, 0.f, 1.f, 0.f, 0.f, 1.f);
    
  
	glGenBuffers(1, &RCAR_vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, RCAR_vbo_);
	glBufferData(GL_ARRAY_BUFFER, RCAR_num_verts_ * sizeof(RCAR_Vertex), &vert_col[0], GL_STATIC_DRAW);   


      }
    
      void render(const float *mvp)
      {
          glUseProgram(RCAR_shader_prog_);
	gl_utils::printGLError(__FILE__, __LINE__);

        glBindBuffer(GL_ARRAY_BUFFER, RCAR_vbo_);
	gl_utils::printGLError(__FILE__, __LINE__);
	glEnableVertexAttribArray(RCAR_pos_loc_);
	gl_utils::printGLError(__FILE__, __LINE__);
	glVertexAttribPointer(RCAR_pos_loc_, 3, GL_FLOAT, GL_FALSE, sizeof(RCAR_Vertex), NULL);
	gl_utils::printGLError(__FILE__, __LINE__);
	glEnableVertexAttribArray(RCAR_col_loc_);
	gl_utils::printGLError(__FILE__, __LINE__);
	glVertexAttribPointer(RCAR_col_loc_, 3, GL_FLOAT, GL_FALSE, sizeof(RCAR_Vertex), BUFF_OFFSET(12));
	gl_utils::printGLError(__FILE__, __LINE__);


	glUniformMatrix4fv(RCAR_mvp_loc_, 1, GL_FALSE, mvp);    
	gl_utils::printGLError(__FILE__, __LINE__);

	glDrawArrays(GL_TRIANGLES, 0, RCAR_num_verts_);

        glBindBuffer(GL_ARRAY_BUFFER, RCAR_vbo_);
        glDisableVertexAttribArray(RCAR_pos_loc_);
        glDisableVertexAttribArray(RCAR_col_loc_);
        glUseProgram(0);
      }

    };

    /** Sets up and renders an axis cross in the lower left corner.
     * \param rc ContextResourceCache in use by the program
     * \param w, h width and height of viewport/window. (used to restore correct size after rendering cross).
     * \param forward_compatible Default = true, creates an openGL 3.0 forward compatible cross.
     */
    inline void renderCoordAxis(const float *mvp, ContextResourceCache *rc, bool forward_compatible)
    {
        if(rc == NULL)
	{
	  return;
	}

      RenderCoordAxisResource *r = 
	static_cast<RenderCoordAxisResource*>(rc->getResource(ContextResourceCache::RESOURCE_RENDERCOORDAXIS));
      if(r == NULL)
	{
	  r = new RenderCoordAxisResource(false);
	  rc->setResource(ContextResourceCache::RESOURCE_RENDERCOORDAXIS, r);
	}
      r->render(mvp);
    }


 

  }//end namespaces
}

