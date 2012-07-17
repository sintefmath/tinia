Tutorial 4: Renderlists{#tut_tutorial4}
===

In this tutorial we build upon [Tutorial 2](@ref tut_tutorial2) and build a more
complex graphical user interface using the Tinia framework.

Familiarity with basic OpenGL and C++ with object orientation is assumed.

The program created in this tutorial will run both as a desktop program and as a
server/client program.

The program consists of three files: The Job class definition and two main files. One
main file will be created for the desktop program, and one for the web program.
We will only highlight changes from [Tutorial 3](@ref tut_tutorial3), so it's a good
idea to brush up on that tutorial before reading this one.

\tableofcontents

Proxy geometry
---
Tinia is designed to be used in a server/client setting. What sets Tinia apart
from other tools, such as remote desktop connections, is that it maintains a
high degree of interactivity even without a high speed, low latency connection.

The interactivity is achieved, in parts, by the use of a proxy geometry on the
client side. The proxy geometry is meant to be a light weight representation
of the geometry being rendered on the server.

### Renderlists
The proxy geometry is defined through the use of a renderlist. The
[OpenGLJob](@ref tinia::jobcontroller::OpenGLJob) class has a virtual method
named [getRenderList](@ref tinia::jobcontroller::OpenGLJob::getRenderList).

The renderlist is represented by an instance of the
[DataBase](@ref tinia::renderlist::DataBase) class. The database will contain
our buffers, it contains our shaders (written in OpenGL ES Shading Language), and
it contains a list of actions (draw order) we want to perform in each renderpass.

The actual use of a renderlist will seem quite similar to that of OpenGL.

### Making our database
First we include an instance of our database in the
[Tutorial4Job](@ref tinia::tutorial::Tutorial4Job) class.
\snippet Tutorial4_Job.hpp mdatabase

For our proxy geometry we'll just create an indentical copy of our triangle,
just drawn using GL_LINES.

First we create a buffer to hold the points we want to draw. We do this by first
creating the buffer, then setting the actual storage using the
[Set](@ref tinia::renderlist::Buffer::Set) method.
\snippet Tutorial4_Job.hpp buffer

We also need to specify our shaders, which will the most simple shader we can
think of:
\snippet Tutorial4_Job.hpp shader

Before we an create our draw order, we need to create actions for each
procedure we want to execute. First we create an action for setting our shader.
We do this by passing in, as a template argument, the action type we'd like.
Then we name our action (in this case "useShader"). The
[createAction](@ref tinia::renderlist::DataBase::createAction) method
returns a pointer to the action created, thus we may invoke the method
[setShader](@ref tinia::renderlist::SetShader::setShader) to specify which shader
we want to use.
\snippet Tutorial4_Job.hpp actionShader

Next we need to setup an action setting the correct inputs. You will notice
that we utilize the fact that most renderlist methods return a pointer to
the object we're working on, thus enabling us to chain commands quite easily. To
specify the buffer, we first set which shader we want to pass the buffer to, then
we just use the [setInput](@ref tinia::renderlist::SetInputs::setInput) method.
Notice that we set the third argument to 3, signalling that we want three components.
\snippet Tutorial4_Job.hpp actionBuffer

We also need to set the MVP matrix. Luckily for us, Tinia provides us with
generated matrices, so we just need to specify which matrix go to our MVP
uniform.
\snippet Tutorial4_Job.hpp actionMVP

And we create a simple draw action
\snippet Tutorial4_Job.hpp actionDraw

Then we set up the draw order
\snippet Tutorial4_Job.hpp drawOrder

And finally we process the databaser
\snippet Tutorial4_Job.hpp process

### Exposing the renderlist
To expose the renderlist to the controller, we need to implement the
[getRenderList](@ref tinia::jobcontroller::OpenGLJob::getRenderList) method as
such:
\snippet Tutorial4_Job.hpp renderlistdecl
\snippet Tutorial4_Job.hpp renderlistfunc

### Initializing an OpenGL extension wrangler
We want to view the renderlist in our desktop program, and to do this we
need to use the `tinia::renderlist::gl` library. This library requires that we
have our extension wrangler initialized, so we need to implement the
[initGL](@ref tinia::jobcontroller::OpenGLJob::initGL) method in order to
initialize GLEW at the right time
\snippet Tutorial4_Job.hpp initGL

### Running the desktop program
To actually view the renderlist in the desktop program, you need to start
the executable with the --renderlist option. In the lower left corner of
the OpenGL canvas, you should have a pull down menu saying "Native Rendering",
changing this to "Render List" should show something similar to this:
\image html tutorial4_desktop.png "Screenshot of the desktop job from Tutorial4."


### Running the web program
If you've successfully installed Tinia you should be able to run the web program
as `tutorial4_web` through the [mod_trell web interface](@ref sec_mod_trell_gui).

The renderlist will show up whenever you hold your mousebutton
down on the canvas. The program should look something like this:
\image html tutorial4_web.png "Screenshot of the web job from Tutorial4."

The full Job file
---
All changes in this tutorial have been done in the Job file of the tutorial:
\include Tutorial4_Job.hpp







