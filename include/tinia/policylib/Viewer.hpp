#ifndef VIEWER_HPP
#define VIEWER_HPP

#include <array>
#include <algorithm>

namespace tinia {
namespace policylib {

/** \struct Viewer
    Viewer is a POD to be used in client code to fetch viewer-data in and out of a Policy.
  */
struct Viewer {

    Viewer() :
        width( 0 ),
        height( 0 ),
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
    std::array<float, 16> projectionMatrix;

    /** Pointer to a modelviewMatrix, assumed to hold 16 continious float values. */
    std::array<float, 16> modelviewMatrix;

    int width;
    int height;
    double timestamp;
    std::string sceneView;
};

}
}
#endif // VIEWER_HPP
