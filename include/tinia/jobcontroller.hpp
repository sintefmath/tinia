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

/** @file jobcontroller.hpp  Convenience header to include all jobcontroller functionality with one include statement.
 *  @ingroup ConvenienceHeaders
 */

#include "tinia/jobcontroller/Controller.hpp"
#include "tinia/jobcontroller/Job.hpp"
#include "tinia/jobcontroller/OpenGLJob.hpp"

/** @namespace tinia::jobcontroller Provides classes necessary to use the \ref JobControllerLibrary. */

/** @page JobControllerLibrary Job Controller
 *
 * The Job Controller framework specifies the interfaces needed to create a Tinia-based
 * application. It has several interfaces depending on the use-case of the
 * application, and can easily be extended if need be. Typical interfaces are
 * QTObserver for a desktop job and TrellObserver for cloud jobs, found in the
 * qtobserver and trell module respectively.
 *
 * A user typically want to subclass either
 * [Job](@ref tinia::jobcontroller::Job) or
 * [OpenGLJob](@ref tinia::jobcontroller::OpenGLJob). To run the program, he then
 * creates a new instance of this subclass, and hands it to the controller using
 * the [setJob](@ref tinia::jobcontroller::Controller::setJob) method, then calls
 * the [run](@ref tinia::jobcontroller::Controller::run) method.
 */
