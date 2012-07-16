Tutorial 2: Specifying GUI (part 1){#tut_tutorial2}
===

In this tutorial we build upon [Tutorial 1](@ref tut_tutorial1) and build a more
complex graphical user interface using the Tinia framework.

Familiarity with basic OpenGL and C++ with object orientation is assumed.

The program created in this tutorial will run both as a desktop program and as a
server/client program.

The program consists of three files: The Job class definition and two main files. One
main file will be created for the desktop program, and one for the web program.
We will only highlight changes from [Tutorial 1](@ref tut_tutorial1), so it's a good
idea to brush up on that tutorial before reading this one.

\tableofcontents

GUI through Tinia
---
The [ExposedModel](@ref tinia::model::ExposedModel) class has the method
[setGUILayout](@ref tinia::model::ExposedModel::setGUILayout) which will be use
to specify how we want our GUI to look.

In the eyes of the model, a GUI is just a tree of widget types defined in the
namespace `tinia::model::gui`. Every GUI starts with a root element. A root element
can be any widget type, but it's usually one of the container widgets
[HorizontalLayout](@ref tinia::model::gui::HorizontalLayout),
[VerticalLayout](@ref tinia::model::gui::VerticalLayout),
[Grid](@ref tinia::model::gui::Grid) or [TabLayout](@ref tinia::model::gui::TabLayout).

Altering Tutorial 1
---
We'd like to modify Tutorial 1 such that it only contains an OpenGL canvas.
First we need to specify the root element, which we choose to be a
[VerticalLayout](@ref tinia::model::gui::VerticalLayout). We make our GUI in
the constructor of [Tutorial2Job](@ref tinia::tutorial::Tutorial2Job).
All GUI elements in
the model are represented as pointers, so we do this as well.

The creation of the [VerticalLayout](@ref tinia::model::gui::VerticalLayout)
is really simple:
\snippet Tutorial2_Job.hpp layout

An OpenGL canvas is represented by a [Canvas](@ref tinia::model::gui::Canvas) element.
The constructor takes the key to the [Viewer](@ref tinia::model::Viewer) as
the first value.
\snippet Tutorial2_Job.hpp canvas

In the previous tutorial we relied on the fact that Tinia defaults the boundinbox
key to "boundingbox", but it's good practice to specify this manually to the
[Canvas](@ref tinia::model::gui::Canvas). This is done with the following line
\snippet Tutorial2_Job.hpp boundingbox

Once we've made our new [Canvas](@ref tinia::model::gui::Canvas) it's just the
simple matter of adding it to the VerticalLayout
\snippet Tutorial2_Job.hpp add

And at last we set our layout as the GUI to the model. Notice how the second argument
is `tinia::model::gui::ALL` which indicates that the GUI could be used
for all types of devices (desktops, mobile devices, tablets):
\snippet Tutorial2_Job.hpp setgui

The rest of the program is left unchanged. The whole `Tutorial2_Job.hpp` is then
\include Tutorial2_Job.hpp

### Ownership of the GUI
The observant reader might have noticed that we don't delete the GUI pointers we've
made in the tutorial. By design, the [ExposedModel](@ref tinia::model::ExposedModel)
takes ownership of the GUI pointers and deletes them upon destruction.

### Running the desktop program
Starting the program should show something similar to this:
\image html tutorial2_desktop.png "Screenshot of the desktop job from Tutorial2."


### Running the web program
If you've successfully installed Tinia you should be able to run the web program
as `tutorial2_web` through the [mod_trell web interface](@ref sec_mod_trell_gui).

The program should look something like this:
\image html tutorial2_web.png "Screenshot of the web job from Tutorial2."



