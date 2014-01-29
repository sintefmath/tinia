Example: Specifying your own viewer controls {#example_fpsviewer}
===

In this tutorial we build upon [Tutorial 3](@ref tut_tutorial3) and 
integrate our own viewer control into a Tinia Application.

Familiarity with basic OpenGL, C++ with object orientation and 
JavaScript is assumed. It is further useful to have a grasp on linear algebra.



Like the other tutorials, this program will run both as a desktop program and as a server/client program.

The program consists of three files: The Job class definition and two main files. One
main file will be created for the desktop program, and one for the web program.

\tableofcontents

Extending Tinias user input
---
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

Simple scene to interact with
---
We must first define a scene which we can interact with. The standard red triangle works fine for a trackball viewer, but is not really suited for FPS interactions.
Instead we will set up a very simple box, where each side has a different color. This can be done in many ways, and the most basic way is shown here:
\snippet tutorial6_job.hpp drawColoredBox


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
All changes in this tutorail have been done in the Job file of the tutorial:
\include tutorial6_job.hpp