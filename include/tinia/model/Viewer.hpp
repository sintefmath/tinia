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

#include <boost/array.hpp>
#include <algorithm>

namespace tinia {
namespace model {

/** \struct Viewer
    Viewer is a POD to be used in client code to fetch viewer-data in and out of a ExposedModel.
  */
struct Viewer {

    Viewer() :
        width( 512 ),
        height( 512 ),
        timestamp( 0.0 ),
        sceneView( "---oooOOOooo---" )
    {
        std::fill( projectionMatrix.begin(), projectionMatrix.end(), 0.0f );
        std::fill( modelviewMatrix.begin(), modelviewMatrix.end(), 0.0f );

        // Making the identity the default
        for(int i = 0; i< 4; i++)
        {
           projectionMatrix[i+4*i] = 1;
           modelviewMatrix[i+4*i] = 1;
        }
    }

    /** Pointer to a projectionMatrix, assumed to hold 16 continious float values. */
    boost::array<float, 16> projectionMatrix;

    /** Pointer to a modelviewMatrix, assumed to hold 16 continious float values. */
	boost::array<float, 16> modelviewMatrix;

    int width;
    int height;
    double timestamp;
    std::string sceneView;
};

}
}

