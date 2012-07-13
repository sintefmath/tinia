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
The constructor takes the key to the [Viewer](@ref tinia::model::gui::Viewer) as
the first value.

