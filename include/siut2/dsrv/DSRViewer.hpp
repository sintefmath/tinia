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
 *  Authors: Erik W. Bjønnes <erik.bjonnes@sintef.no>
 *           Christopher Dyken <christopher.dyken@sintef.no>
 *
 *  This file is part of the siut library.
 *  Copyright (C) 2009 by SINTEF.  All rights reserved.
 *

 *  SINTEF, Pb 124 Blindern, N-0314 Oslo, Norway
 *  http://www.sintef.no
 *********************************************************************/
/**
 * \file DSRViewer.hpp
 *
 * \section Usage
 * DSRViewer is an graphics API agnostic viewer class that handles
 * rotations, translations, creating modelview, projection,
 * modelviewprojection matrices, as well as their inverse matrices.
 * It support get/set states for single states, or the entire viewer state.
 * \section Requires
 * In addition to Freeglut, GLEW and GLM  siut2/dsrv/math, siut2/exceptions, and siut2/perf_utils are also required.

 */

#pragma once


#include <glm/gtc/type_ptr.hpp> //for c_ptr equvialent
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp> //for rotate quat with vec3
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include "siut2/dsrv/math/BBox.hpp"
#include "siut2/perf_utils/FpsCounter.hpp"
#include "siut2/dsrv/math/Utils.hpp"


#define DEBUG_VIEW

#ifdef DEBUG_VIEW
#include <cmath>
#define THROW_VIEW
#include <stdexcept>
#endif

#ifndef M_PI
#define M_PI 3.1415926535
#endif

namespace siut2
{
    /** \brief A tool for quickly getting a light-weight renderer up and running.
     * Gives you a freeglut context with openGL 3.x context for light-weight rendering.
     * Most functions are written to be forward_compatible, though it can be used with OpenGL 2.x as well (not fully tested).
     *
     * \section Requirements:
     * Freeglut 2.6.0 or later (build from source might be best solution, since new releases are *very* slow)
     * GLEW 1.5.1* or later. Latest build used GLEW 1.5.8 which was default on ubuntu and easily available on windows
     * GLM 0.9.1 Download from \linkhttp://glm.g-truc.net\endlink
     *
     * Simple example file for usage is located in examples/simpleViewer/sv_main.cpp
     *
     *
     * \section TODO
     * 1. Change from using a bounding sphere/box to find the camera position and center of interest to using actual camera position and coi. Will make it easier to implement fly-mode, working zoom into objects etc.
     * 1.1 Fix zoom bug (tied with 1).
     * 2. Clean up even more of the #defines.
     * 3. Add methods for use with XML commands instead of mouse input
     */
    namespace dsrv
    {
        /** Viewer class that keeps calculates the modelview, projection, and modelviewProjection matrices, as well as their inverses.
         * Also handles rotations, translations, panning, dollying and zooming in a numerically stable way.
         * Supports saving and loading of states from strings.
         */
        class DSRViewer
        {
        public:

            /** Camera states.
             *
             * The following  states are available in this implementation: */
            enum
                {
                    /** Nothing happening, camera standing still. */
                    NONE,
                    /** Rotate a set distance. */
                    ROTATE,
                    /** Rotate and keep spinning in the same direction. */
                    SPIN,
                    /** Pan camera. */
                    PAN,
                    /** Dolly camera, keeps center of interest. */
                    DOLLY,
                    /** Move camera forwards/backwards, adjusting for up, moves center of interest. */
                    FOLLOW,
                    /** Zoom in/out on scene. */
                    ZOOM,
                    /** Rotate about viewer position. */
                    ORIENT
                };

            /** Projection states.
             * Supports both Perspective projection and orthographic projection.*/
            enum ProjectionType
                {
                    PROJECTION_PERSPECTIVE,
                    PROJECTION_ORTHOGRAPHIC
                };

            /** Creates a new viewer focused on the lerped center of the bounding box.
             *
             * Sets view volume to be the AABB, with perspective projection.
             * Window size is set to (1,1) and field of view is set to 90 degrees (glm needs degrees instead of radians).
             * Calls viewAll()
             */
            DSRViewer(const glm::vec3 &bb_min, const glm::vec3 &bb_max)
                : m_projection(PROJECTION_PERSPECTIVE),
                  m_state( NONE ),
                  m_spin_factor(1)
            {
                printDebug = false;
                m_camera_state_curr.m_fov = 90.f; //since glm requires degrees instead of radians
                setViewVolume(bb_min, bb_max);
                m_win_size[0] = 1; m_win_size[1] = 1; //safeguard
                viewAll();
            }


            /** Destructor, does nothing. */
            ~DSRViewer()
            {
                ;
            }

            /** Method that decreases the spin-factor by 0.1f.
             * Default spin-factor = 1.0f*/
            void decreaseSpin()
            {
                m_spin_factor -= 0.1f;
            }

            /** Method that increases the spin-factor by 0.1f.
             * Default spin-factor = 1.0f*/
            void increaseSpin()
            {
                m_spin_factor += 0.1f;
            }

            /** Sets new camera state.
             *
             * Called by the window manager to set a new camera state.
             * Saves mouse position, and reset elapsed time.
             * Saves camera state
             * \param int state state of camera, NONE, SPIN, etc.
             * \param int x, y mouse position
             */
            void startMotion(int state, int x, int y)
            {
                endMotion(x, y);

                m_mouse_state_curr.m_mouse_pos = glm::vec2(static_cast<float>(x), static_cast<float>(y));
                m_mouse_state_curr.m_elapsed = 0.f;
                m_mouse_state_init = m_mouse_state_prev = m_mouse_state_curr;
                m_timer.restart();

                m_camera_state_init = m_camera_state_curr;
                if(state == ROTATE) {
                    m_state = ROTATE;
                }
                else if(state == PAN) {
                    m_state = PAN;
                }
                else if(state == DOLLY) {
                    m_state = DOLLY;
                }
                else if( state == FOLLOW ) {
                    m_state = FOLLOW;
                    m_up_axis = upAxis( m_camera_state_init.m_orientation );
                    m_camera_state_init.m_orientation = removeRoll( m_camera_state_init.m_orientation, m_up_axis );
                    m_camera_state_curr.m_orientation = m_camera_state_init.m_orientation;
                }
                else if(state == ZOOM) {
                    m_state = ZOOM;
                }
                else if( state == ORIENT ) {
                    m_state = ORIENT;
                    m_up_axis = upAxis( m_camera_state_init.m_orientation );
                    m_camera_state_init.m_orientation = removeRoll( m_camera_state_init.m_orientation, m_up_axis );
                    m_camera_state_curr.m_orientation = m_camera_state_init.m_orientation;
                }
            }

            /** Motion function called by the window manager.
             *
             * Calculates the values needed for the current camera state, NONE, SPIN, etc.
             * Then calls calculateMatrices()
             * \param int x, y the new mouse position.
             */
            void motion(int x, int y)
            {
                float elapsed = m_timer.elapsed();
                m_timer.restart();
                motion(x, y, elapsed);
            }

            void motion(int x, int y, float elapsed)
            {
                m_mouse_state_prev = m_mouse_state_curr;
                m_mouse_state_curr.m_mouse_pos = glm::vec2(static_cast<float>(x), static_cast<float>(y));
                m_mouse_state_curr.m_elapsed = static_cast<float>(m_timer.elapsed());
                m_timer.restart();

                if(m_state == ROTATE) {
                    glm::vec3 axis;
                    float angle;
                    if(trackball(axis, angle, m_mouse_state_init.m_mouse_pos, m_mouse_state_curr.m_mouse_pos)) {
                        glm::quat rot;
                        float sinA = static_cast<float>(std::sin(0.5*angle));
                        float cosA = static_cast<float>(std::cos(0.5*angle));
                        axis = glm::normalize(axis);
                        rot.w = cosA;
                        rot.x = sinA*axis[0];
                        rot.y = sinA*axis[1];
                        rot.z = sinA*axis[2];
                        m_camera_state_curr.m_orientation = rot * (m_camera_state_init.m_orientation);
                    }
                    else {
                        m_camera_state_curr.m_orientation = m_camera_state_init.m_orientation;
                    }
                    calculateMatrices();
                }
                else if( m_state == ORIENT ) {
                    glm::vec2 d =
                            normDevFromWinCoords( m_win_size, m_mouse_state_curr.m_mouse_pos )-
                            normDevFromWinCoords( m_win_size, m_mouse_state_init.m_mouse_pos );

                    glm::quat rot =
                            glm::angleAxis(-45.f*d[0], glm::vec3(0.f, 1.f, 0.f) ) *
                            glm::angleAxis( 45.f*d[1], glm::vec3(1.f, 0.f, 0.f) );

                    m_camera_state_curr.m_orientation = removeRoll( rot*m_camera_state_init.m_orientation, m_up_axis );
                    glm::vec3 v = glm::conjugate( m_camera_state_init.m_orientation ) * glm::vec3( 0.f, 0.f, -m_camera_state_init.m_distance );
                    glm::vec3 w = glm::conjugate( m_camera_state_curr.m_orientation ) * glm::vec3( 0.f, 0.f, -m_camera_state_curr.m_distance );
                    m_camera_state_curr.m_center_of_interest =
                            m_camera_state_init.m_center_of_interest + (w - v);

                    calculateMatrices();
                }
                else if(m_state == PAN) {
                    glm::vec3 ms_mouse_a, ms_mouse_b;
                    ms_mouse_a = mousePosOnInterestPlaneAsObjectCoords(normDevFromWinCoords(m_win_size, m_mouse_state_init.m_mouse_pos));
                    ms_mouse_b = mousePosOnInterestPlaneAsObjectCoords(normDevFromWinCoords(m_win_size, m_mouse_state_curr.m_mouse_pos));
                    m_camera_state_curr.m_center_of_interest = m_camera_state_init.m_center_of_interest - (ms_mouse_b - ms_mouse_a);
                    calculateMatrices();
                }

                else if(m_state == DOLLY) {
                    glm::vec4 plane = glm::vec4(0.f, 0.f, -1.f, 1.f) * m_camera_state_init.m_model_view_projection;
                    plane.w = -glm::dot(glm::vec3(plane.x, plane.y, plane.z), m_camera_state_init.m_center_of_interest);
                    if(plane.w != plane.w) {
                        return;
                    }
                    glm::vec2 a_nd = normDevFromWinCoords(m_win_size, m_mouse_state_init.m_mouse_pos);
                    glm::vec2 b_nd = normDevFromWinCoords(m_win_size, m_mouse_state_curr.m_mouse_pos);

                    glm::vec3 d = getPointOnPlaneFromNormDevCoords(m_camera_state_init.m_model_view_projection, plane, b_nd) -
                        getPointOnPlaneFromNormDevCoords(m_camera_state_init.m_model_view_projection, plane, a_nd);

                    const glm::mat4 &m = m_camera_state_init.m_model_view;
                    glm::vec3 x = glm::vec3(-m[0][0], m[0][1], m[0][2]);
                    glm::vec3 z = glm::vec3(m[2][0], m[2][1], m[2][2]);

                    m_camera_state_curr.m_center_of_interest = m_camera_state_init.m_center_of_interest - glm::dot(x, d) * z;
                    calculateMatrices();
                }
                else if( m_state == FOLLOW ) {
                    glm::vec2 d =
                            normDevFromWinCoords(m_win_size, m_mouse_state_curr.m_mouse_pos) -
                            normDevFromWinCoords(m_win_size, m_mouse_state_init.m_mouse_pos);

                    glm::vec3 dir = glm::conjugate(m_camera_state_init.m_orientation) * glm::vec3( 0.f, 0.f, 1.f );
                    dir = glm::normalize( dir - glm::dot(dir,m_up_axis)*m_up_axis );
                    m_camera_state_curr.m_center_of_interest =
                            m_camera_state_init.m_center_of_interest +
                            d[1] * m_view_volume.getRadius() * dir;

/*

                    MV = T(D)R(ORIENT)T(-COI)
m_camera_state_curr.m_model_view = glm::translate( glm::mat4(), cam_pos );
                    m_camera_state_curr.m_model_view = m_camera_state_curr.m_model_view * rotmatrix;
                    m_camera_state_curr.m_model_view = glm::translate( m_camera_state_curr.m_model_view,
                                                                       -m_camera_state_curr.m_center_of_interest );
*/

                    calculateMatrices();

                }
                else if(m_state == ZOOM) {
                    glm::vec2 d =
                            normDevFromWinCoords(m_win_size, m_mouse_state_curr.m_mouse_pos) -
                            normDevFromWinCoords(m_win_size, m_mouse_state_init.m_mouse_pos);

                    m_camera_state_curr.m_distance = m_camera_state_init.m_distance * (1 + d[1]);

                    float distTemp =  0.01f * m_view_volume.getRadius();

                    if(m_camera_state_curr.m_distance < distTemp)
                        {
                            m_camera_state_curr.m_distance = distTemp;
                        }
                    calculateMatrices();
                }
            }
            /** If state is SPIN, calculates new positions and call calculateMatrices()
             */
            void update()
            {

                if(m_state == SPIN)
                    {
                        //doing manual rotation instead of glm since it avoids conversion to and from degrees.
                        glm::quat ori = m_camera_state_curr.m_orientation;
                        glm::quat spin;
                        float angle = m_spin_speed * m_spin_factor * static_cast<float>(m_timer.elapsed());
                        float sinA = static_cast<float>(std::sin(0.5*angle));
                        float cosA = static_cast<float>(std::cos(0.5*angle));
                        m_spin_axis = glm::normalize(m_spin_axis);
                        spin.w = cosA;
                        spin.x = sinA*m_spin_axis[0];
                        spin.y = sinA*m_spin_axis[1];
                        spin.z = sinA*m_spin_axis[2];
                        m_camera_state_curr.m_orientation = spin * m_camera_state_curr.m_orientation;
                        if(checkNaN(m_camera_state_curr.m_orientation))
                            {
                                m_camera_state_curr.m_orientation = ori;
                            }
                        m_timer.restart();
                        calculateMatrices();

                    }

            }

            /** Updates mouse state and updates state to SPIN or NONE.
             *
             * Saves mouse state and resets timer.
             * If state is ROTATE and time since last call is less than 0.1 set state to SPIN, else to NONE.
             */
            void endMotion(int x, int y)
            {

                float elapsed = m_timer.elapsed();
                m_timer.restart();
                if (m_state != NONE) {
                    endMotion(x, y, elapsed);
                }

            }
            void endMotion(int x, int y, float elapsed)
            {
                MouseState curr;
                curr.m_mouse_pos = glm::vec2(static_cast<float>(x), static_cast<float>(y));
                curr.m_elapsed = static_cast<float>(elapsed);
                if(m_state == ROTATE) {
                    if(false && curr.m_elapsed < 0.5 ) {
                        std::cout << "Consider spin\n";
                        float angle;
                        if(trackball(m_spin_axis, angle, m_mouse_state_prev.m_mouse_pos,
                                     curr.m_mouse_pos))
                            {
                                m_spin_speed = angle / (m_mouse_state_prev.m_elapsed + m_mouse_state_curr.m_elapsed +
                                                        curr.m_elapsed);
                                m_state = SPIN;
                                std::cout << "Do spin\n";
                            }
                    }
                }
                if (m_state != SPIN && m_state != NONE) {
                    motion(x, y, elapsed);
                    m_state = NONE;
                }

            }

            /** Sets the projection type to be used for the viewer
             */
            void setProjectionType(ProjectionType pt)
            {
                m_projection = pt;

                calculateMatrices();
            }

            /** Returns the projection type being used by the viewer
             */
            ProjectionType getProjectionType() const
            {
                return m_projection;
            }

            /** Sets the window size to be used by the viewer
             */
            void setWindowSize(int w, int h)
            {
                m_win_size = glm::vec2(static_cast<float>(w), static_cast<float>(h));
                calculateMatrices();
            }

            /** Sets the orientation of the camera to that of the matrix.
             *
             * Does this by making the matrix into a quaternion
             */
            void setOrientation(const glm::mat4 &m)
            {
                m_state = NONE;
                m_camera_state_curr.m_orientation = glm::quat(m);
                calculateMatrices();
            }

            /** Sets the orientation of the camera to that of the quaternion.
             */
            void setOrientation(const glm::quat &q)
            {
                m_state = NONE;
                m_camera_state_curr.m_orientation = q;

                calculateMatrices();
            }

            /** Sets the view volume of the viewer to an axis aligned bounding box give by the input parameters.
             *
             * \param[in] bb_min the min corner of the AABB.
             * \param[in] bb_max the max corner of the AABB.
             * Calls calculateMatrices()
             */
            void setViewVolume(const glm::vec3 &bb_min, const glm::vec3 &bb_max)
            {

                m_view_volume = BBox(bb_min, bb_max);
                viewAll();
                calculateMatrices();
            }

            /** Sets the camera from an affine transformation and a projection matrix.
              *
              * It uses the rotation and translation parts of the modelview
              * matrix to position and orient the camera, and uses the angle
              * between the top and bottom frustum planes to deduce the
              * field-of-view along y.
              *
              * If guess_bbox is enabled, it positions the center of interest
              * in the center between the near and far planes and sets the
              * bounding box to be an axis aligned box centered around this
              * point with side lengths equal to the distance between the near
              * and far planes.
              *
              * \param modelview  An homogeneous affine transform matrix that
              *                   encodes the position and orientation of the camera.
              * \param projection An homogeneous projection matrix.
              *
              * \author Christopher Dyken, <christopher.dyken@sintef.no>
              */
            void
            setCamera( const glm::mat4& modelview, const glm::mat4& projection, const bool guess_bbox = false )
            {

                // Calculate fov and guess projection type
                glm::vec4 tp = glm::row( projection, 3 ) - glm::row( projection, 1 );
                glm::vec4 bp = glm::row( projection, 3 ) + glm::row( projection, 1 );
                float fov_y = std::acos( glm::dot( glm::normalize( glm::vec3( tp.x, tp.y, tp.z ) ),
                                                   glm::normalize( glm::vec3( bp.x, bp.y, bp.z ) ) ) );

                if( fov_y > 0.1 ) {
                    m_projection = PROJECTION_PERSPECTIVE;
                    m_camera_state_curr.m_fov = (180.f/M_PI)*fov_y;
                }
                else {
                    // todo
                }

                // extract linear transformation
                glm::mat3 A = upperLeft3x3( modelview );
                glm::mat3 Ai = glm::inverse( A );

                // extract rotation
                glm::mat3 R;
                polarDecomposition( R, A, A );
                m_camera_state_curr.m_orientation = glm::quat_cast( R );

                // extract observer position, calc coi, and transform to world
                glm::vec3 op = -translation( modelview );

                if( guess_bbox ) {
                    glm::vec4 np = glm::row( projection, 3 ) + glm::row( projection, 2 );
                    glm::vec4 fp = glm::row( projection, 3 ) - glm::row( projection, 2 );
                    float n = np.w/glm::length( glm::vec3( np.x, np.y, np.z ) );
                    float f = -fp.w/glm::length( glm::vec3( fp.x, fp.y, fp.z ) );
                    float d = 0.5f*(std::abs(f)-std::abs(n));
                    m_camera_state_curr.m_distance = -0.5f*(n+f);

                    glm::vec3 mb = glm::vec3( 0.f, 0.f, m_camera_state_curr.m_distance);
                    m_camera_state_curr.m_center_of_interest = Ai*(op - mb );

                    m_view_volume = BBox( m_camera_state_curr.m_center_of_interest - glm::vec3(d),
                                          m_camera_state_curr.m_center_of_interest + glm::vec3(d) );
                }
                else {
                    glm::vec3 mb = glm::vec3( 0.f, 0.f, m_camera_state_curr.m_distance);
                    m_camera_state_curr.m_center_of_interest = Ai*(op - mb );
                }

                calculateMatrices();
            }


            /**Updates the view volume, but does not move the camera to look at whole scene
             *
             */
            void updateViewVolume(const glm::vec3 &bb_min, const glm::vec3 bb_max)
            {
                m_view_volume = BBox(bb_min, bb_max);
                calculateMatrices();
            }

            /** Sets the viewer to have the entire AABB in view.
             *
             * Updates camera center of interest and distance.
             * Calls calculateMatrices()
             */
            void viewAll()
            {
                m_camera_state_curr.m_center_of_interest = 0.5f*( m_view_volume.getMin() + m_view_volume.getMax() );
                m_camera_state_curr.m_distance = m_view_volume.getRadius() / tanf(m_camera_state_curr.m_fov*(M_PI/360.f));
                calculateMatrices();
            }

            /** Returns the Modelview matrix. */
            const glm::mat4& getModelviewMatrix() const
            {
                return m_camera_state_curr.m_model_view;
            }

            /** Returns the Modelview inverse Matrix. */
            const glm::mat4& getModelviewInverseMatrix() const
            {
                return m_camera_state_curr.m_model_view_inverse;
            }

            /** Returns the Projection matrix. */
            const glm::mat4& getProjectionMatrix() const
            {
                return m_camera_state_curr.m_projection;
            }

            /** Returns the inverse Projection matrix.*/
            const glm::mat4& getProjectionInverseMatrix() const
            {
                return m_camera_state_curr.m_projection_inverse;
            }

            /** Returns the ModelviewProjection matrix. */
            const glm::mat4& getModelviewProjectionMatrix() const
            {
                return m_camera_state_curr.m_model_view_projection;
            }

            /** Returns the inverse ModelviewProjection. */
            const glm::mat4& getModelviewProjectionInverseMatrix() const
            {
                return m_camera_state_curr.m_model_view_projection_inverse;
            }

            /** Returns the orientation of the camera. */
            const glm::quat& getOrientation() const
            {
                return m_camera_state_curr.m_orientation;
            }

            /** Returns the center of interest. */
            const glm::vec3& getCenterOfInterest() const
            {
                return m_camera_state_curr.m_center_of_interest;
            }

            /** Returns the window size used by the viewer. */
            const glm::vec2& getWindowSize() const
            {
                return m_win_size;
            }


            float getAspectRatio() const
            {
                return m_win_size[0]/m_win_size[1];
            }

            float getFieldOfViewX() const
            {
                return getAspectRatio()*m_camera_state_curr.m_fov;
            }

            float getFieldOfViewY() const
            {
                return m_camera_state_curr.m_fov;
            }

            /** Returns the near and far planes in a vec2.
             *
             * Only returns the distance from the camera into the view volume.
             */
            glm::vec2 getNearFarPlanes() const
            {
                return glm::vec2( m_camera_state_curr.m_near,
                                  m_camera_state_curr.m_far );
            }


            /** Returns the current viewpoint, ie. position of camera
             *
             * Takes this from the inverse Modelview matrix.
             */
            glm::vec3 getCurrentViewPoint() const
            {
                glm::mat4 inv =  m_camera_state_curr.m_model_view_inverse;

                return glm::vec3(glm::column(inv, 3) );
            }


            /** Struct containing all relevant states for DSRV
             *  Used to get/set the state of DSRV
             */
            struct DsrvState
            {
                BBox m_view_volume;
                glm::vec2 m_window_size;
                DSRViewer::ProjectionType m_projection_type;
                glm::vec3 m_camera_center_of_interest;
                float m_camera_field_of_view;
                glm::quat m_camera_orientation;
                glm::mat4 m_camera_projection;
                glm::mat4 m_camera_projection_inverse;
                glm::mat4 m_camera_model_view;
                glm::mat4 m_camera_model_view_inverse;
                glm::mat4 m_camera_model_view_projection;
                glm::mat4 m_camera_model_view_projection_inverse;

                DsrvState()
                {
                    m_view_volume = BBox();
                    m_window_size = glm::vec2();
                    m_projection_type = PROJECTION_PERSPECTIVE;
                    m_camera_center_of_interest = glm::vec3();
                    m_camera_field_of_view = 0.f;
                    m_camera_orientation = glm::quat();
                    m_camera_projection = glm::mat4();
                    m_camera_projection_inverse = glm::mat4();
                    m_camera_model_view = glm::mat4();
                    m_camera_model_view_projection = glm::mat4();
                    m_camera_model_view_inverse = glm::mat4();
                    m_camera_model_view_projection_inverse = glm::mat4();
                }
            };

            /** Returns the state of DSRViewer
             */
            DSRViewer::DsrvState* getState() const
            {
                DsrvState *return_state = new DsrvState();
                return_state->m_view_volume = m_view_volume;
                return_state->m_window_size = m_win_size;
                return_state->m_projection_type = m_projection;
                return_state->m_camera_center_of_interest = m_camera_state_curr.m_center_of_interest;
                return_state->m_camera_field_of_view = m_camera_state_curr.m_fov;
                return_state->m_camera_orientation = m_camera_state_curr.m_orientation;
                return_state->m_camera_projection = m_camera_state_curr.m_projection;
                return_state->m_camera_projection_inverse = m_camera_state_curr.m_projection_inverse;
                return_state->m_camera_model_view = m_camera_state_curr.m_model_view;
                return_state->m_camera_model_view_inverse = m_camera_state_curr.m_model_view_inverse;
                return_state->m_camera_model_view_projection = m_camera_state_curr.m_model_view_projection;
                return_state->m_camera_model_view_projection_inverse = m_camera_state_curr.m_model_view_projection_inverse;

                return return_state;
            }

            /** Set the state of the DSRViewer
             *  Sets the full state of the DSRViewer based on the struct passed in.
             */
            void setState(DSRViewer::DsrvState const* ds)
            {
                m_view_volume = ds->m_view_volume;
                m_win_size = ds->m_window_size;
                m_projection = ds->m_projection_type;
                m_camera_state_curr.m_center_of_interest = ds->m_camera_center_of_interest;
                m_camera_state_curr.m_fov = ds->m_camera_field_of_view;
                m_camera_state_curr.m_orientation = ds->m_camera_orientation;
                m_camera_state_curr.m_projection = ds->m_camera_projection;
                m_camera_state_curr.m_projection_inverse = ds->m_camera_projection_inverse;
                m_camera_state_curr.m_model_view = ds->m_camera_model_view;
                m_camera_state_curr.m_model_view_inverse = ds->m_camera_model_view_inverse;
                m_camera_state_curr.m_model_view_projection = ds->m_camera_model_view_projection;
                m_camera_state_curr.m_model_view_projection_inverse = ds->m_camera_model_view_projection_inverse;

            }

            /** Struct keeping track of the camera states.
             *
             * Saves the matrices in use, as well as the orientation, coi, distance and fov.
             */
            struct CameraState
            {
                glm::mat4 m_model_view;
                glm::mat4 m_model_view_inverse;
                glm::mat4 m_projection;
                glm::mat4 m_projection_inverse;
                glm::mat4 m_model_view_projection;
                glm::mat4 m_model_view_projection_inverse;
                glm::quat m_orientation;
                glm::vec3 m_center_of_interest;
                float     m_near;
                float     m_far;
                float     m_distance;
                float     m_fov;
            };

            /** Returns the camera states.
             */
            CameraState* getCameraState() const
            {
                return new CameraState(m_camera_state_curr);
            }

            /** Sets the camera states
             */
            void setCameraState(CameraState const* c)
            {
                m_camera_state_init = *c;
                m_camera_state_curr = m_camera_state_init;
            }

            /** Set the field of view
             * \param fov in degrees
             * Converts the input fov to radians and stores it in the class.
             */
            void setFOV(const float fov)
            {
                m_camera_state_curr.m_fov = fov;
            }

            void prettyprintProjectionMatrices()
            {
#if 0
                fprintf(stderr, "%f, %f, %f, %f\n", m_camera_state_curr.m_projection[0][0],m_camera_state_curr.m_projection[0][1],m_camera_state_curr.m_projection[0][2],m_camera_state_curr.m_projection[0][3]);
                fprintf(stderr, "%f, %f, %f, %f\n", m_camera_state_curr.m_projection[1][0],m_camera_state_curr.m_projection[1][1],m_camera_state_curr.m_projection[1][2],m_camera_state_curr.m_projection[1][3]);
                fprintf(stderr, "%f, %f, %f, %f\n", m_camera_state_curr.m_projection[2][0],m_camera_state_curr.m_projection[2][1],m_camera_state_curr.m_projection[2][2],m_camera_state_curr.m_projection[2][3]);
                fprintf(stderr, "%f, %f, %f, %f\n", m_camera_state_curr.m_projection[3][0],m_camera_state_curr.m_projection[3][1],m_camera_state_curr.m_projection[3][2],m_camera_state_curr.m_projection[3][3]);

                fprintf(stderr, "\n\n%f, %f, %f, %f\n", m_camera_state_curr.m_projection_inverse[0][0],m_camera_state_curr.m_projection_inverse[1][0],m_camera_state_curr.m_projection_inverse[2][0],m_camera_state_curr.m_projection_inverse[3][0]);
                fprintf(stderr, "%f, %f, %f, %f\n", m_camera_state_curr.m_projection_inverse[0][1],m_camera_state_curr.m_projection_inverse[1][1],m_camera_state_curr.m_projection_inverse[2][1],m_camera_state_curr.m_projection_inverse[3][1]);
                fprintf(stderr, "%f, %f, %f, %f\n", m_camera_state_curr.m_projection_inverse[0][2],m_camera_state_curr.m_projection_inverse[1][2],m_camera_state_curr.m_projection_inverse[2][2],m_camera_state_curr.m_projection_inverse[3][2]);
                fprintf(stderr, "%f, %f, %f, %f\n", m_camera_state_curr.m_projection_inverse[0][3],m_camera_state_curr.m_projection_inverse[1][3],m_camera_state_curr.m_projection_inverse[2][3],m_camera_state_curr.m_projection_inverse[3][3]);
#endif
            }

            bool printDebug;

        protected:
            /** Struct for keeping track of mouse states.
             *
             * Contains position and time elapsed since last update
             */
            struct MouseState
            {
                glm::vec2 m_mouse_pos;
                float m_elapsed;
            };
            glm::vec3 m_up_axis;

            /** Return the cardinal axis that most closely map to the camera's y-axis.
             *
             * \param q  The rotation that takes world coords to camera coords.
             * \returns The cardinal axis.
             * \author Christopher Dyken, <christopher.dyken@sintef.no>
             */
            static inline glm::vec3 upAxis( const glm::quat& q )
            {
                glm::vec3 y_w = glm::conjugate( q )*glm::vec3( 0.f, 1.f, 0.f );
                glm::vec3 a_w = glm::abs( y_w );
                if( (a_w.x > a_w.y) && (a_w.x > a_w.z ) ) {
                    return glm::vec3( y_w.x > 0.f ? 1.f : -1.f, 0.f, 0.f );
                }
                else if( a_w.y > a_w.z ) {
                    return glm::vec3( 0.f, y_w.y > 0.f ? 1.f : -1.f, 0.f );
                }
                else {
                    return glm::vec3( 0.f, 0.f, y_w.z > 0.f ? 1.f : -1.f );
                }
            }

            /** Removes camera roll by forcing the camera x-axis to be orthogonal to up.
              *
              * \param q  The rotation that takes world coords to camera coords.
              * \param up The world-space up-axis, normalized.
              * \returns The quaternion q with roll removed.
              * \author Christopher Dyken, <christopher.dyken@sintef.no>
              */
            static inline glm::quat removeRoll( const glm::quat& q, const glm::vec3& up )
            {
                glm::vec3 x_w = glm::conjugate( q ) * glm::vec3( 1.f, 0.f, 0.f );
                glm::vec3 t_w = glm::normalize( x_w - glm::dot(x_w,up)*up );
                glm::quat r = greatCircle( x_w, t_w );
                return q*glm::conjugate(r);
            }

            /** Creates the great-circle rotation from a to b.
              * \param a The 'from'-vector.
              * \param b The 'to'-vector.
              * \returns The quaternion q with roll removed.
              * \author Christopher Dyken, <christopher.dyken@sintef.no>
              */
            static inline glm::quat greatCircle( const glm::vec3& a, const glm::vec3& b )
            {
                float t = glm::dot( a, b );
                if( std::fabs(t) < 0.999f ) {
                    float p = (180.f/M_PI)*std::acos(t);
                    glm::vec3 v = glm::normalize( glm::cross(a,b) );
                    return glm::angleAxis( p, v );
                }
                else {
                    return glm::quat();
                }
            }

            /** Returns a point on the unit sphere. */
            glm::vec3 getPointOnUnitSphere(glm::vec2 p)
            {
                float r2 = p.x*p.x + p.y * p.y;
                if(r2 < 0.25f)
                    {
                        return glm::vec3(2.f * p[0], 2.f * p[1], std::sqrt(1.f-4.f*r2));
                    }
                else
                    {
                        float r = 1.f / std::sqrt(r2);
                        return glm::vec3(r * p[0], r * p[1], 0.f);
                    }
            }

            /** Calculates the matrices used by the viewer.
             *
             * Calculates the correct modelview, projection, modelviewprojection matrices, and their inverse,
             * for the current projection.
             */
            void calculateMatrices()
            {

                glm::vec3 cam_pos = glm::vec3( 0.f, 0.f, -m_camera_state_curr.m_distance );
                glm::mat4 rotmatrix = glm::mat4_cast( m_camera_state_curr.m_orientation );

                // modelview matrix
                m_camera_state_curr.m_model_view = glm::translate( glm::mat4(), cam_pos );
                m_camera_state_curr.m_model_view = m_camera_state_curr.m_model_view * rotmatrix;
                m_camera_state_curr.m_model_view = glm::translate( m_camera_state_curr.m_model_view,
                                                                   -m_camera_state_curr.m_center_of_interest );
                // create inverse modelview matrix
                m_camera_state_curr.m_model_view_inverse = glm::translate(glm::mat4(), m_camera_state_curr.m_center_of_interest);
                m_camera_state_curr.m_model_view_inverse *= glm::mat4_cast(glm::conjugate(m_camera_state_curr.m_orientation));
                m_camera_state_curr.m_model_view_inverse *= glm::translate(glm::mat4(), -cam_pos);

                // determine near_ and far
                float near_, far_;
                glm::vec3 bbmin = m_view_volume.getMin();
                glm::vec3 bbmax = m_view_volume.getMax();

                for(int i=0; i<8; i++) {
                    glm::vec4 p = glm::vec4( (i&1)==0 ? bbmin.x : bbmax.x,
                                             (i&2)==0 ? bbmin.y : bbmax.y,
                                             (i&4)==0 ? bbmin.z : bbmax.z,
                                             1.f );
                    glm::vec4 h = m_camera_state_curr.m_model_view * p;
                    float d = (1.f/h.w)*h.z;
                    if( i == 0 ) {
                        near_ = far_ = d;
                    }
                    else {
                        near_ = std::max( near_, d );
                        far_  = std::min( far_,  d );
                    }
                }

                // use infty-norm of bbox to get an idea of the scale of the scene, and
                // determine an appropriate epsilon
                float e = std::max( std::numeric_limits<float>::epsilon(),
                                    0.001f*std::max( bbmax.x-bbmin.x,
                                                     std::max( bbmax.y-bbmin.y,
                                                               bbmax.z-bbmin.z )
                                                     )
                                    );
                far_  = std::min( -e, far_-e );
                near_ = std::min( 0.01f*far_, std::max(far_, near_ + e ) );
                m_camera_state_curr.m_near = near_;
                m_camera_state_curr.m_far = far_;

                float aspect = m_win_size[0]/m_win_size[1];
                if(m_projection == PROJECTION_PERSPECTIVE) {
                    //would like to use our own projection matrix function, since it will remove the need for using degrees
                    m_camera_state_curr.m_projection = glm::perspective(m_camera_state_curr.m_fov, aspect, -near_, -far_ );
                    m_camera_state_curr.m_projection_inverse = glm::inverse(m_camera_state_curr.m_projection);
                }
                else if(m_projection == PROJECTION_ORTHOGRAPHIC) {
                    float h = tan(0.5f * m_camera_state_curr.m_fov) * m_camera_state_curr.m_distance;
                    float w = aspect * h;
                    m_camera_state_curr.m_projection = glm::ortho(-w, w, -h, h, -near_, -far_);
                    m_camera_state_curr.m_projection_inverse = glm::inverse(m_camera_state_curr.m_projection);
                }
                m_camera_state_curr.m_model_view_projection = m_camera_state_curr.m_projection * m_camera_state_curr.m_model_view;
                m_camera_state_curr.m_model_view_projection_inverse = m_camera_state_curr.m_model_view_inverse * m_camera_state_curr.m_projection_inverse;

            }

            /** If true gives the axis and angle between the two points.
             *
             * \param axis between points, return value
             * \param angle between the points, return value
             * \param wc1 point in window coordinates
             * \param wc2 second point in window coordinates
             */
            bool trackball(glm::vec3 &axis, float &angle, const glm::vec2 &wc1, const glm::vec2 &wc2)
            {
                const glm::vec3 sphere1 = getPointOnUnitSphere( glm::vec2((wc1[0] / m_win_size[0] -0.5f),
                                                                          -wc1[1] / m_win_size[1]+0.5f));

                const glm::vec3 sphere2 = getPointOnUnitSphere( glm::vec2((wc2[0] / m_win_size[0] -0.5f),
                                                                          -wc2[1] / m_win_size[1]+0.5f));

                // note: length of cross product is proportional to sine of
                // angle, i.e., a trigonometric func can be removed (dyken).

                angle = acos( glm::dot( sphere1, sphere2 ) );
                if( fabs( angle ) < std::numeric_limits<float>::epsilon() ) {
                    return false;
                }
                else {
                    axis = glm::normalize( glm::cross( sphere1, sphere2 ) );
                    if(axis.x != axis.x || axis.y != axis.y || axis.z != axis.z)
                        {
                            //degenerate axis from normalised cross product of two identical vectors/spheres.
                            return false;
                        }
                    return true;
                }

            }

            /** Checks if any of the elements of a quaternion is NaN by checking if any element != itself
             *
             * \param Quaternion to check
             */
            inline bool checkNaN(glm::quat q)
            {
                return ((q.x != q.x) || (q.y != q.y) || (q.z != q.z) || (q.w != q.w));
            }

            /**
             *
             */
            inline glm::vec3 mousePosOnInterestPlaneAsObjectCoords( glm::vec2 mouse_pos)
            {
                //find ndc-depth of center of interest plane
                glm::vec4 h = glm::vec4(m_camera_state_init.m_center_of_interest.x,
                                                 m_camera_state_init.m_center_of_interest.y,
                                                 m_camera_state_init.m_center_of_interest.z,
                                                 1.f);
                h = m_camera_state_init.m_model_view_projection * h;
                float z = h.z/h.w;

                //Build ndc mouse position at found depth and project back to object space.
                h.x = mouse_pos.x;
                h.y = mouse_pos.y;
                h.z = z;
                h.w = 1.f;
                h = m_camera_state_init.m_model_view_projection_inverse * h;

                return (1.f/h.w)*glm::vec3(h.x, h.y, h.z);

            }



            MouseState m_mouse_state_init, m_mouse_state_prev, m_mouse_state_curr;
            CameraState m_camera_state_init, m_camera_state_curr;

            ProjectionType m_projection;
            int m_state;
            glm::vec3 m_spin_axis;
            float m_spin_speed;
            float m_spin_factor;
            perf_utils::FpsCounter m_timer;
            BBox m_view_volume;
            glm::vec2 m_win_size;
        };
    }

}
