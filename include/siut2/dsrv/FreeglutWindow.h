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

/* -*- mode: C++; tab-width:4; c-basic-offset: 4; indent-tabs-mode:nil -*- */
/************************************************************************
 *
 *  File: DSRViewer.hpp
 *
 *  Created: 12. August 2009
 *
 *  Version: $Id: $
 *
 *  Authors: Erik W. Bjoennes <erik.bjonnes@sintef.no>
 *           Christopher Dyken <christopher.dyken@sintef.no>
 *
 *  This file is part of the siut library.
 *  Copyright (C) 2009 by SINTEF.  All rights reserved.
 *

 *  SINTEF, Pb 124 Blindern, N-0314 Oslo, Norway
 *  http://www.sintef.no
 *********************************************************************/
/**
  * \file FreeglutWindow.hpp
  *
  * \section Usage
  * FreeglutWindow is a win
  * \section Requires
  * siut2 is required
  */
#pragma once

#include "siut2/dsrv/DSRViewer.hpp"
#include "siut2/dsrv/math/BBox.hpp"
#include "siut2/perf_utils/FpsCounter.hpp"

#include "siut2/gl_utils/GLSLtools.hpp"
#include "siut2/gl_utils/RenderText.hpp"
#include "siut2/gl_utils/RenderCoordAxis.hpp"

#include <GL/glew.h>
#include <GL/freeglut.h>


namespace siut2
{
namespace dsrv
{
  /** Window manager class that uses Freeglut and OpenGL to give you a rendering context, with helper axis-cross and fps info.
   * Contains minimum needed to get a window up and running, allowing you to focus on the graphics part.
   * Is meant to be sub classed  by each application, so that it stays a light-weight class.
   */
  class FreeglutWindow
  {
    
  public:
	  /** Constructor, does partial setup of the window manager.
     * Calls glutInit with argc and argv.
     * Will call setUpContext() with title if set_up_later is false. 
     * \param argc, argv parameters for glutInit
     * \param set_up_later set to true if you want deferred context creation.
     * \param title the title of the window, used if you are not using deferred context creation.
     */ 
	  FreeglutWindow();

    /** Destructor, empty. */
    virtual ~FreeglutWindow() {}

    
    /** Returns the viewer used.
     */
    DSRViewer* getDSRView(int i);

    /** Starts the fps timer and calls glutMainLoop(). */
    void run();

    /** Function to enable/disable rendering of fps and info. */
    void setFpsVisibility(bool visible);
	/** Enable/Disable rendering of coordinate system */
	void setCoordSysVisibility( bool visible );

    /** Sets the size of the window. */
    void setSize(int width, int height);  

    /** Sets the color of the text used for rendering fps and info */
    void setTextColor(const glm::vec3 &color);

    /** Sets up a Freeglut and GLEW forward-compatible context, registers callbacks.
	* @param title the title of the window.
	* @param display_mode the display mode you want to create, bitmask of modes.
	*/
	void setUpContext(int *argc, char **argv, const char *title, unsigned int display_mode=0);

	/**Sets up the Freeglut and GLEW context, registers callbacks. Supports mixed-mode.
	* @param title the title of the window
	* @param display_mode the display mode you want to create, bitmask of modes.
	*/
	void setUpMixedModeContext(int *argc, char **argv, const char *title, unsigned int display_mode=0);

  protected:
	  static FreeglutWindow* m_instance;// needed since Freeglut requires static callback methods

    gl_utils::ContextResourceCache*  rescache_;

    /** Pointer to the viewer used by the window manager.
     * Is a pointer, since it should be easy to update for use with multiple view ports, and thus viewers, at a later date.
     */
    DSRViewer *m_viewer;

    /** Boolean used to determine if fps info should be printed in the render window or not. */
    bool m_fps_visible;

    bool m_coordsys_visible;

    /** Used by setUpContext to make sure that it's not being called twice.
     * This could happen if you create an instance of FreeglutWindow with setup_later = true. */
    bool m_context_set_up;

    /** Clock used for calculating fps. */
    perf_utils::FpsCounter m_fps_omega;

    /** Number of frames since last update. */
    unsigned int m_fps_frames;

    /** String containing number of fps since last update, as well as the string returned from info(). */
    std::string m_fps_string;

    /** Vec3 containing the color of the text used for the fps info. 
     * Could be deprecated, depending on how we go about rendering strings in the future. */
    glm::vec3 m_text_color;

    /** Sets up the viewer and sets the window size. 
     * Should be called by the inheriting class after constructor, but before using any functions to make sure
     * the viewer is properly set up.
	 * \Param[in] min_ Min corner of AABB.
	 * \Param[in] max_ Max corner of AABB.
     */
    virtual void init(glm::vec3 &min_, glm::vec3 &max_);

    /** Idle function, calls glutPostRedisplay().
     * Called by idlefunc(), subclass to add own commands
     */
    virtual void idle();

    /** Returns a string of information you want to be displayed next to the FPS counter.
     * Override for use.
     * Not working until we have support for displaying text...
     */
    virtual std::string info();   

    /** Called by keyboardFunc() whenever a key is pressed.
     * Do your processing of keyboard inputs when overriding this in an inheriting class.
     * Just empty shell here.
     */
    virtual void keyboard(unsigned char key);

    /** Render function called by displayFunc(). Override with your rendering routine in subclass.
     * Empty shell.
     *
     */
    virtual void render();

    /** Called by reshapeFunc() whenever window is resized.
     * Handles resizing the window, setting the view port and updating the viewer.
     * If overridden, don't forget to call this one as well ;)
     */
    virtual void reshape(int width, int height);

    /** Handles special keys inputs (F1-F12 keys and such), called by specialFunc().
     * Override to add your own commands.
     */
    virtual void special(int key, int x, int y);

    /** Called by Freeglut to draw the scene.
     * Resets the view port, clears depth and color buffers.
     * Calls update() on viewer, before calling render(), which is the one to override.
     * Has functionality for setting up, and showing fps info, if m_fps_visible == true;
     */
    static void displayFunc();

	/** Same as displayFunc() only with support for fixed function rendering as well
	*
	*/
	static void displayMixedFunc();


    /** Called by freeglut whenever there is no work to do. Calls idle(); */
    static void idleFunc();

    /** Called by freelgut to handle keyboard input, and calls again keyboard(), which is the one to override.
     * \param key button pressed
     * \param x, y mouse position
     * Calls glutPostRedisplay()
     */
    static void keyboardFunc(unsigned char key, int x, int y);

    /** Handles motion input from freeglut, calling motion() */
    static void motionFunc(int x, int y);

    /** Handles mouse inputs, calling viewer->newState with state either ROTATE, PAN, ZOOM or NONE.
     * \param b button
     * \param s state
     * \param x, y screen coordinates of where it happened
     * Calls glutPostRedisplay()
     */
    static void mouseFunc(int b, int s, int x, int y);

    /** Handles mouse wheel interactions, calling viewer with new state and motion with DOLLY state.
     * \param w wheel
     * \param d direction moved
     * \param x, y mouse position
     * Calls glutPostRedisplay() 
     */
    static void mouseWheelFunc(int w, int d, int x, int y);

    /** Called by freeglut when window size changes. If both values are positive, calls reshape().
     * \param w, h width and height of new window size/
     */
    static void reshapeFunc(int width, int height);

    /** Called by freeglut when special keys (F1-F12 and others) are pressed, and calls special().
     * \param key button pressed
     * \param x, y mouse position  
     * Calls glutPostRedisplay()
     */
    static void specialFunc(int key, int x, int y);
  };
}
}

