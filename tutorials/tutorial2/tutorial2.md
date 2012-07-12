Tutorial 2: Specifying GUI {#tut_tutorial2}
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


