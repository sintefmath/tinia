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
 * along with the Tinia Framework.  If not, se <http://www.gnu.org/licenses/>.
 */

#pragma once

/** @file renderlist.hpp  Convenience header to include all renderlist functionality with one include statement.
 *  @ingroup ConvenienceHeaders
 */

#include "tinia/renderlist/Action.hpp"
#include "tinia/renderlist/Buffer.hpp"
#include "tinia/renderlist/DataBase.hpp"
#include "tinia/renderlist/Draw.hpp"
#include "tinia/renderlist/Image.hpp"
#include "tinia/renderlist/Logger.hpp"
#include "tinia/renderlist/RenderList.hpp"
#include "tinia/renderlist/SetFramebuffer.hpp"
#include "tinia/renderlist/SetFramebufferState.hpp"
#include "tinia/renderlist/SetInputs.hpp"
#include "tinia/renderlist/SetLight.hpp"
#include "tinia/renderlist/SetLocalCoordSys.hpp"
#include "tinia/renderlist/SetPixelState.hpp"
#include "tinia/renderlist/SetRasterState.hpp"
#include "tinia/renderlist/SetShader.hpp"
#include "tinia/renderlist/SetUniforms.hpp"
#include "tinia/renderlist/SetViewCoordSys.hpp"
#include "tinia/renderlist/Shader.hpp"
#include "tinia/renderlist/XMLWriter.hpp"

/** @namespace tinia::renderlist Provides classes necessary to use the \ref RenderListLibrary. */

/** @page RenderListLibrary RenderList
 *  RenderList is a library used for building and manipulating OpenGL and WebGL state.
 *
 * In Tinia, the RenderList is mainly used to represent a proxy geometry. This proxy
 * geometry is supposed to be a light weight representation of a heavy geometry.
 *
 * The main component in the RenderList library is the
 * [DataBase](@ref tinia::renderlist::DataBase) class. The DataBase consists of
 * buffers, shaders, actions and a draw order.
 *
 * For a nice introduction on how to use renderlists, consult [Tutorial 4](@ref tut_tutorial4).
 */
