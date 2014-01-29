Tutorial 6: Implementing a FPS viewer {#tut_tutorial6}
===

In this tutorial we build upon [Tutorial 3](@ref tut_tutorial3) and 
integrate our own viewer control into a Tinia Application.

Familiarity with basic OpenGL, C++ with object orientation and 
JavaScript is assumed. It is further useful to have a grasp on linear algebra.



Like the other tutorials, this program will run both as a desktop program and as a server/client program.

The program consists of four files: The Javascript file, the Job class definition and two main files. One
main file will be created for the desktop program, and one for the web program.

\tableofcontents

Extending Tinias user input
====
Whilst Tinia comes with support for inspecting a scene/object through a trackball viewer, it is often useful to be able to navigate the camera around the scene.
So in this tutorial we will extend the viewer controls of Tinias renderwindow to support FPS-style navigation of the scene.

This then requires writing new _event handler_ methods that reacts to the user input, be it key presses, mouse movements etc., and translate them into camera movements.
We assume that you know how to set up a job in Tinia, if not please read the other tutorials, and will concentrate on the changes needed to implement the FPS camera.


~~To keep the GUI as responsive as possible for the end user, these methods 
should be run as "close" to the user as possible. In a web-application, this
requirement mandates that the user input is processed client-side in the web
browser. And hence they have to be implement in Javascript, Tinia includes
as Javascript engine, so that these methods can be reused in standalone desktop
applications.~~

Setting up a simple scene to interact 
===
We must first define a scene which we can interact with. The standard red triangle works fine for a trackball viewer, but is not really suited for FPS interactions.
Instead we will set up a very simple box, where each side has a different color. This can be done in many ways, and the most basic way is shown here:
\snippet tutorial6_job.hpp drawColoredBox

When you add that to your overloaded
[renderFrame](@ref tinia::jobcontroller::OpenGLJob::renderFrame) function we will get a nice box on the screen. If it looks a bit strange, make sure that the depth test is enabled and the depth buffer properly cleared.
If you try to interact with it now, you will see the normal behaviour of the trackball viewer.

Creating our own Matrices
===
To change how the camera works we need to add Javascript code that can calculate the ModelView and Projection matrices required by Tinia.
We will not touch on the details behind the math of these two matrices as that is beyond the scope of this tutorial, but suffice to say the ```modelView``` matrix is the one that rotates and moves the camera whereas the ```projection``` matrix projects the scene onto the screen.
If you are interested in more details about them, the following resources might be of interest:
www.graphicscodex.com, http://mathworld.wolfram.com/ProjectionMatrix.html, and http://www.songho.ca/opengl/gl_transform.html


The initial javascript file
--
We must first set up a Javascript file that we can add the relevant code to. This will need a constructor where we can get the relevant variables from Tinia, and variables to store our ModelView and Projection matrices.
So lets set up theadd the following declaration:
  
    function Tutorial6FPSViewer( params ) {
        this._model = params.exposedModel;
        this._key = params.key;
        this._bboxKey = params.boundingBoxKey;

        this._modelView = mat4.identity( mat4.create() );
        this._projection = mat4.identity( mat4.create() );
        
        this._bbMin = vec3.create();
        this._bbMax = vec3.create();

        var viewer = this._model.getElementValue( this._key );
        this._width = viewer.getElementValue( "width" );
        this._height = viewer.getElementValue( "height" );
        this._aspect = this._width / this.height;        
    }

With this we have the basics needed to start implementing our FPS viewer, but if you try to use this now it will not work.
First we need to calculate the projection matrix, and send the update value back to the server. 
The ModelView matrix can be left as an identity matrix for now, meaning any input will be ignored. 

Calculating the Perspective Projection Matrix
---
To make our models appear on the screen, we need to use the Perspective Projection matrix, which will make it look like a road leading away look like a point the closer it gets to the horizon.
There are many ways with which to calculate this matrix, and we will use the one found in the ```OpenGL Red Book``` which also corresponds to the one you get when calling the gl-matrix.js library.
The mathematical formula is \f[projection = \begin{pmatrix}\frac{near*2}{right-left}& 0& \frac{right + left}{right - left}& 0\\ 0& \frac{near*2}{top - bottom}& \frac{top + bottom}{top - bottom}& 0\\0& 0& -\frac{far + near}{far - near}& -\frac{far * near * 2}{far - near} \\0& 0& -1& 0 \end{pmatrix}\f]
As you can seee this is not a very scary formula, the only question is how do we get the values for far, near, top, bottom, left right? You can of course set them to arbitrary values, but it makes much more sense to tailor them to the scene you are looking at.
This can be done relatively easily by using the bounding box of the scene, finding the points furthest and closest to the camera, and then calculate the rest of the values from this and the aspect ratio.

We can "steal" the relevant code to do this from (@ref DSRV.js):

      // --- set up projection matrix

        // the eight corners of the bounding box
        var corners = [[bbmin[0], bbmin[1], bbmin[2], 1.0],
                       [bbmin[0], bbmin[1], bbmax[2], 1.0],
                       [bbmin[0], bbmax[1], bbmin[2], 1.0],
                       [bbmin[0], bbmax[1], bbmax[2], 1.0],
                       [bbmax[0], bbmin[1], bbmin[2], 1.0],
                       [bbmax[0], bbmin[1], bbmax[2], 1.0],
                       [bbmax[0], bbmax[1], bbmin[2], 1.0],
                       [bbmax[0], bbmax[1], bbmax[2], 1.0]];
        // apply the modelview matrix to the eight corners to get the minimum
        // and maximum z in the camera's local coordinate system.
        var near, far;
        for (i in corners) {
            var c = corners[i];
            var p = mat4.multiplyVec4(this.m_modelview, c);
            //           window.console.log( p );
            var z = (1.0 / p[3]) * p[2];
            if (near == null) {
                near = z;
                far = z;
            }
            else {
                near = Math.max(near, z);
                far = Math.min(far, z);
            }
        }

The minimal javascript file
---
We must first define which events we want to listen to, by defining
methods the Javascript file that we want to reimplement. The parameters object
passed to the constructor contains the Exposed Model and can get and set
values in the model, which will be communicated back to the server.

In our FPS Viewer code, we reimplement method that 
accept an _event_ parameter, and communicates back to the model by setting 
the projection and model-view matrices accordingly.

Injecting the Javascript code into our application
---






The full Job file
---
All changes in this tutorial have been done in the Job file of the tutorial:
\include tutorial6_job.hpp

The full Javascript file
---
All the Javascript code in this tutorial can be found in the .js file of the tutorial
\include tutorial6.js