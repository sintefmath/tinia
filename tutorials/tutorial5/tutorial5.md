Tutorial 5: Adding your own viewer controls {#tut_tutorial5}
============================================================

In this tutorial we will build a very simple drawing program that
let us color pixel in a texture when we click the mouse over them.
This requires the user to write his own _event handler_ methods that 
reacts to user input such as mouse movement and mouse clicks.

To keep the GUI as responsive as possible for the end user, these methods 
should be run as "close" to the user as possible. In a web-application, this
requirement mandates that the user input is processed client-side in the web
browser. Hence they have to be implement in Javascript, Tinia includes
as Javascript engine, so that these methods can be reused in standalone desktop
applications.

It is assumed that the reader has read [Tutorial 3](@ref tut_tutorial3) and
is familiar with how to implement listeners. Furthermore, the the reader
should have a basic understanding of JavaScript.

The program will consist of four files, two separate files for containing
the main()-function for either the desktop or web-version. Thereafter we 
have the TextureDrawer-class definition and finally a small Javascript class that
handles our mouse-clicks and passes them to the Exposed Model.


The Job and Listener class
--------------------------
The Job class will be responsible for drawing a texture to the screen,
as well as reciving mouse-clicks that will update the texture.

To keep the  simple, we let the Job class inherit both 
[OpenGLJob](@ref tinia::jobcontroller::OpenGLJob) and 
[StateListener](@ref tinia::model::StateListener). 
\snippet tutorial5_job.hpp declaration

This gives us three virtual methods to override:
\snippet tutorial5_job.hpp overrides

Not surprisingly, these methods have been overridden in several of the
previous examples, and they will have to be reimplemented yet again.
Since we are a little more advanced OpenGL in this tutorial, we must init
the OpenGL subsystem, in our case we are using glew. We also set up the
texture so it can be used in the renderloop later.

\snippet tutorial5_job.hpp initGL

Getting mouse events
--------------------
To handle mouse-events we must implment our own viewer-class that
handles input from the user. These event-handlers must be implemented in 
Javascript, and the canvas must be told which Viewertype to use. 
By default, Tinia applications uses a trackball viewer, which we have seen in the previous 
tutorials.

The event handlers communicates to the server by reading and setting 
values in the ExposedModel.

The Javascript file is very short, and its full contents are given below:
\include tutorial5.js
As we can see, we introduce a Javascript-class called MouseClickResponder with just
a couple of few methods. These methods update the key "click_xy" with
the window positions of the mouse click.

We must now tell the Tinia application which viewer to use. This happens
in the constructor of the TextureDrawer class.
\snippet tutorial5_job.hpp canvas
Notice that the string we give as argument to the [canvas](@ref tinia::model::gui::Canvas)
is the same as the name of the class in the Javascript file.

Finally we set up the the "click_xy" element in the model, and registers
the TextureDrawer class to handle these events.
\snippet tutorial5_job.hpp setupclick
Since we are interested in passing a two-component vector as parameter 
through the model, we have to store it as a string. Better support for
complex types in the model will come in a future Tinia release.

Handling mouse events and updating the texture
----------------------------------------------
Since TextureDrawer is registered as a Listener, it recives a notification
in the stateElementModified-method whenever "click_xy" is updated. This fetches
the update "click_xy" coordinate (as a string!) from the model, and then 
applies a series of coordinate transforms on it, until we have a coordinate
in the image-space which we can update.

\snippet tutorial5_job.hpp stateElementModified 

Running the program
-------------------

You should now be able to launch the program like in the previous tutorials.

As you click in the window in either application, you will color
that pixel white. We have now a bare bones pixel editor up and running!

\image html tutorial5_desktop.png "Screenshot of the desktop job from Tutorial 5".

\image html tutorial5_web.png "Screenshot of the web job from Tutorial 5".


The complete TextureDrawer file
-------------------------------
\include tutorial5_job.hpp

The complete Javascript file
----------------------------
\include tutorial5.js








