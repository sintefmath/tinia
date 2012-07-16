Tutorial 3: Specifying GUI (part 2){#tut_tutorial5}
===

In this tutorial we build upon [Tutorial 2](@ref tut_tutorial2) and build a more
complex graphical user interface using the Tinia framework.

Familiarity with basic OpenGL and C++ with object orientation is assumed.

The program created in this tutorial will run both as a desktop program and as a
server/client program.

The program consists of three files: The Job class definition and two main files. One
main file will be created for the desktop program, and one for the web program.
We will only highlight changes from [Tutorial 2](@ref tut_tutorial2), so it's a good
idea to brush up on that tutorial before reading this one.

\tableofcontents

User input through Tinia
---
With the exception of layout and spacing widgets (e.g.
[HorizontalLayout](@ref tinia::model::gui::HorizontalLayout) and
[HorizontalExpandingSpace](@ref tinia::model::gui::HorizontalExpandingSpace))
most GUI widgets in Tinia passes user information to the exposed model.

A [TextInput](@ref tinia::model::gui::TextInput) for instance, takes the text
the user has entered in and hands it over to the exposed model. The exposed
model is then free deny the text (if for example the element is an integer
the model will only accept text that is convertible to integers). If the exposed
model accepts the new value, the relevant components will be notified. Specifically,
an update to an element in the exposed model will trigger a redraw of the OpenGL
canvas.

Making our triangle resizeable
---
We want to make our triangle from the previous example resizeable. To be precise,
we want to define three scalars \f$s_1, s_2, s_3\in [0,10]\f$, and define the
three corners of our triangle to be
\f[v_1 = \begin{pmatrix}0\\ 0\\ 0\end{pmatrix} - s_1 \begin{pmatrix}1\\ 0\\ 0\end{pmatrix}\f]
\f[v_2 = \begin{pmatrix}1\\ 0\\ 0\end{pmatrix} + s_2 \begin{pmatrix}1\\ 0\\ 0\end{pmatrix}\f]
\f[v_3 = \begin{pmatrix}1\\1\\ 0\end{pmatrix} + s_2 \begin{pmatrix}1\\ 0\\ 0\end{pmatrix} + s_3 \begin{pmatrix}0\\ 1\\ 0\end{pmatrix}\f]
The two corners of the boundingbox will then be
\f[\{\begin{pmatrix}-s_1\\ 0\\ 0\end{pmatrix}, \begin{pmatrix}1+s_2\\ 1 + s_3\\ 0\end{pmatrix}\}\f].

Constrained elements in ExposedModel
---
A constrained element in [ExposedModel](@ref tinia::model::ExposedModel) is an element
with upper and lower bounds. For our triangle example, we need to add \f$s_1, s_2, s_3\f$
as constrained elements via the [addConstrainedElement](@ref tinia::model::ExposedModel::addConstrainedElement) method.
The first argument to this method is the key, the second is the current value, the third is the minimum allowed
value for the element, the fourth is the maximum allowed value for the element.
\snippet Tutorial5_Job.hpp constrained

Notice that we add our elements as `int`, and the model is able to deduce the
type automatically.

Listeners to ExposedModel
---
If we allow the user to resize the triangle, we need to update our boundingbox
whenever the user updates either \f$s_1\f$, \f$s_2 \f$ or \f$s_3\f$. To do this,
we want to add a simple listener to the ExposedModel. A listener here is just
a subclass of `tinia::model::StateListener` with the method
[stateElementModified](@ref tinia::model::StateListener::stateElementModified) implemented.

Our listener class is this simple class:

\snippet Tutorial5_Job.hpp listenerdef

First we need to get a hold of the Exposed model, which we receive in the constructor
\snippet Tutorial5_Job.hpp listenerctor

Then in the constructor of [Tutorial5Listener](@ref tinia::tutorial::Tutorial5Listener)
we add ourselves as a listener to the relevant elements using the
[addStateListener](@ref tinia::model::ExposedModel::addStateListener) method
\snippet Tutorial5_Job.hpp addlistener

Once we've added ourselves as a listener, we must also ensure that we remove ourselves
upon deletion of the listener, hence we need the following destructor in the listener
\snippet Tutorial5_Job.hpp removelistener

Finally we write the [stateElementModified](@ref tinia::model::StateListener::stateElementModified)
method. This method firsts gets the three scalars, then create the new boundingbox as a string
and lastly updates the boundingbox to the model.
\snippet Tutorial5_Job.hpp stateelementmodified

We store our listener in the [Tutorial5Job](@ref tinia::tutorial5::Tutorial5Job)
class in the variable
\snippet Tutorial5_Job.hpp mlistener

In the constructor of [Tutorial5Job](@ref tinia::tutorial5::Tutorial5Job) we
instantiate the listener
\snippet Tutorial5_Job.hpp clistener

Sliders and labels
---
We want to modify \f$s_1\f$, \f$s_2\f$ and \f$s_3\f$ through a slider. To use a slider
in Tinia, simply add a new instance of [HorizontalSlider](@ref tinia::model::gui::HorizontalSlider)
to the GUI layout. The [HorizontalSlider](@ref tinia::model::gui::HorizontalSlider)
accepts the key of the element as the first parameter to the constructor.
\snippet Tutorial5_Job.hpp slider

To the left of each slider we want a label saying either "Left corner", "Right corner" or "Upper corner".
The [Label](@ref tinia::model::gui::Label) lets us add labels to each element, but we need
to add annotations to each element for it to display anything more useful than
"s1", "s2" or "s3", hence we use the method [addAnnotation](@ref tinia::model::ExposedModel::addAnnotation)
in the model
\snippet Tutorial5_Job.hpp annotation

Then we create the three labels
\snippet Tutorial5_Job.hpp label

Finally create a [Grid](@ref tinia::model::gui::Grid) layout with size 3 times 3
to hold our six widgets plus 3
[HorizontalExpandingSpace](@ref tinia::model::gui::HorizontalExpandingSpace)s spacers.
Finally we add the grid
to the main layout:
\snippet Tutorial5_Job.hpp grid

Modification to the renderloop
---
We only need to make small modifications to the renderloop. First we need to get
the scalars
\snippet Tutorial5_Job.hpp getscalars

then we utilize the scalars while drawing the triangle
\snippet Tutorial5_Job.hpp renderloop

### Running the desktop program
Starting the program should show something similar to this:
\image html tutorial5_desktop.png "Screenshot of the desktop job from Tutorial5."


### Running the web program
If you've successfully installed Tinia you should be able to run the web program
as `tutorial5_web` through the [mod_trell web interface](@ref sec_mod_trell_gui).

The program should look something like this:
\image html tutorial5_web.png "Screenshot of the web job from Tutorial5."

The full Job file
---
All changes in this tutorail have been done in the Job file of the tutorial:
\include Tutorial5_Job.hpp







