Tutorial 5: Specifying your own viewer controls {#tut_tutorial5}
===

In this tutorial we build upon [Tutorial 3](@ref tut_tutorial3) and 
integrate our own viewer control into a Tinia Application.

Familiarity with basic OpenGL, C++ with object orientation and 
JavaScript is assumed.

We will implement Nehe Camera tutorial.



The program created in this tutorial will run both as a desktop program and as a
server/client program.

The program consists of three files: The Job class definition and two main files. One
main file will be created for the desktop program, and one for the web program.
We will only highlight changes from [Tutorial 3](@ref tut_tutorial3), so it's a good
idea to brush up on that tutorial before reading this one.

\tableofcontents

Extending Tinias user input
---
The goal of this tutorial is to extend the viewer controls of Tinias renderwindow
to all for FPS-style navigation in the scene.

This requires the user to write his own _event handler_ methods that 
reacts to user input such as keypresses, mouse movement and mouse clicks.

To keep the GUI as responsive as possible for the end user, these methods 
should be run as "close" to the user as possible. In a web-application, this
requirement mandates that the user input is processed client-side in the web
browser. And hence they have to be implement in Javascript, Tinia includes
as Javascript engine, so that these methods can be reused in standalone desktop
applications.


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
\include Tutorial5_Job.hpp