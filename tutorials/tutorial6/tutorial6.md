Tutorial 6: Implementing a FPS viewer in Tinia {#tut_tutorial6}
===

In this tutorial we build upon [Tutorial 3](@ref tut_tutorial3) and integrate our own viewer control into a Tinia Application.
Familiarity with basic OpenGL, C++ with object orientation and JavaScript is assumed. It is further useful to have a grasp on linear algebra.

We will first go over how you connect a new viewer to the Tinia Framework, how you select it from the C++ side, and what you need to do in Javascript before implementing the neccessary math.

Like the other tutorials, this program will run both as a desktop program and as a server/client program.
The program consists of four files: The Javascript file, the Job class definition and two main files. One main file will be created for the desktop program, and one for the web program.
These complete files are located under /tutorials/tutorial6/, and can also be viewed at the bottom of this tutorial.

\tableofcontents

Extending Tinias user input
====
Whilst Tinia comes with support for inspecting a scene/object through a trackball viewer, we realise this is not always the ideal solution and have taken steps to make it easy to create other types of viewers.
And so in this tutorial we will extend the viewer controls of Tinias renderwindow to support FPS-style navigation of the scene.

This then requires writing new _event handler_ methods that reacts to the user input, be it key presses, mouse movements etc., and translate them into camera movements.
We assume that you know how to set up a job in Tinia, if not please read the other tutorials, and will concentrate on the changes needed to implement the FPS camera.

To keep the GUI as responsive as possible for the end user, these methods should be run as "close" to the user as possible.
In a web-application, this requirement mandates that the user input is processed client-side in the web browser and hence have to be implemented in Javascript.
And since Tinia supports both web- and desktop-applications, we have included a Javascript engine in Tinia so the Javascript viewer also works for desktop-applications.


Mapping between Tinia and Javascript code
---
In [Tutorial 5](@ref tut_tutorial5) we saw how to create our own listeners in C++ for changes to a variable that is updated from Javascript.
It is also possible to do this the other way, binding functions in Javascript to be called when a variable is changed (either by resizing the window or changes on the server).
And the latter is needed when we want to implement a new viewer.

Before we start implementing the changes needed for our new viewer, we should save our the variables we need to deal with from Tinia. 
So in our empty Javascript file add the following:

    function FPSViewer( params ) {
        console.log( "Constructing FPSViewer" );
        this.m_model = params.exposedModel;
        this.m_key = params.key;
        this.m_bbKey = params.boundingBoxKey;

        var viewer = this.m_model.getElementValue( this.m_key );
        this.m_width = viewer.getElementValue( "width" );
        this.m_height = viewer.getElementValue( "height" );
        this.m_aspect = this.m_width / this.m_height;
    }

This will give us access to the exposedModel where Tinia stores all the variables exposed to the client, they key to our viewer and the key to the boundingbox so we can access them later.
You also see how we can access the viewer to get variables stored there, in this case the height and width.

For our viewer we need to listen for changes to the size of the window we are rendering to, as well as changes to the boundingbox since we will be basing our projection on it. 
To do this we simply need to add the following code to the bottom of the constructor we wrote above:

    this.m_exposedModel.addLocalListener(this.m_boundingBoxKey, tinia.hitch(this, function(key, bb) {
        this.updateBoundingBox(bb);
        this.calculateProjectionMatrix();
        this.insertMatrices();
    }));

    this.m_exposedModel.addLocalListener(this.m_key, tinia.hitch(this, function(key, viewer) {
        var height = viewer.getElementValue("height");
        var width = viewer.getElementValue("width");
        if(height !== this.m_height || width !== this.m_width) {
            this.m_height = height;
            this.m_width = width;
            this.m_aspect = width / height;
            this.calculateProjectionMatrix();
            this.insertMatrices();
        }
    }));

The first part tells Tinia that when the bounding box is changed we want to call 3 other functions. `this.updateBoundingBox(bb);` will store the updated bounding box so we can easily access it later.
`this.calculateProjectionMatrix();` will create and store a new projection matrix based on the updated bounding box, and `this.insertMatrices();` will send the updated matrix back to the server.

The second part is quite similar, except that instead of updating the bounding box it will update the height, width and aspect ratio of our window and then create, store and send the updated matrix. 

We will leave how to calculate the projection matrix for later in the tutorial, and focus now on how `this.insertMatrices();` tells Tinia we have made a change and it needs to give us an updated image from the server.

Similar to how we registered our listeners, Tinia automatically registers listeners for certain variables that triggers a [renderFrame](@ref tinia::jobcontroller::OpenGLJob::renderFrame) call.
So in `this.insertMatrices();` what we do is copy our local \f$modelView\f$ and \f$projection\f$ matrices into the corresponding variables and after it has been called Tinia will automatically dispatch a request for a new image to display based on the updated matrices. 

    FPSViewer.prototype = {
    //we should keep our viewer functions inside the prototype, except the constructor.
        insertMatrices: function () {
    
            var viewer = this.m_exposedModel.getElementValue(this.m_key);
            viewer.updateElement("modelview", this.m_modelView);
            viewer.updateElement("projection", this.m_projection);
            this.m_exposedModel.updateElement(this.m_key, viewer);
        }
    }

Now all that is needed to do is to have Tinia load our Javascript and tell it to use it as the current viewer.
This is done in C++, and assuming we have already loaded the script into memory and know the viewers name, can be done by the following lines:

    //done where you set up the IPCGLController or QtController.
    controller.addScript( viewerSource ); 

    //done where you set up the canvas
    canvas->setViewerType( std::string("MyViewer") ); //where "MyViewer" is the name of the viewer you have loaded.
    
With this in place your viewer should be up and ready for interactions!


Creating the FPS viewer
====
Now that we know what we need to do to get our viewer to work with Tinia, lets implement a FPS viewer using this knowledge.

First we are going to need a scene to interact with. The standard red triangle used in the other tutorials works fine for a trackball viewer, but is not really suited for FPS interactions.
(I tried, and got rather confused rather quickly!)

If you already have a scene you want to interact with, please use that, for the rest, we can use this code snippet rendering simple box with differently colored sides.
\snippet tutorial6_job.hpp drawColoredBox 

Add this to your overloaded [renderFrame](@ref tinia::jobcontroller::OpenGLJob::renderFrame) function and you should get a nice box on the screen. If it looks a bit strange, make sure that the depth test is enabled and the depth buffer properly cleared.
(This is done using `glEnable(GL_DEPTH_TEST);` and `glClear( GL_DEPTH_BUFFER_BIT );`)


Calculating the Perspective Projection Matrix
----
In our listeners we called the function `this.calculateProjectionMatrix();` and now we should look closer at how it can be implemented.
We will not really touch on the details behind it, as that is way outside the scope of this tutorial, but suffice to say the \f$projection\f$ matrix is responsible for taking our 3D world and projecting it onto our 2D screen.
If you are interested in more details about it, you will find quite a bit at the following resources: [MathWorld](http://mathworld.wolfram.com/ProjectionMatrix.html), [Song Ho](http://www.songho.ca/opengl/gl_transform.html) and for more details see [this Pixar paper]()

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
            //do the perspective division
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

        var epsilon = 0.001;
        far = Math.min(-epsilon, far - epsilon);
        // don't let near get closer than 0.01 of far, and make sure near is
        // closer than far
        near = Math.min(0.01 * far, Math.max(far, near + epsilon));

        this._projection = mat4.perspective( 90, this._aspect, -near, -far );

With this code we have found the near and far plane based on the scenes bounding box and are now ready to create our projection matrix. 
The gl-matrix library can do the heavy lifting for us if we call `mat.perspective( verticalFieldOfView, aspectRatio, -near, -far )`, otherwise all we need to do is calculate top, bottom, left and right and plug it into the \f$projection \ matrix\f$ formula.
We negate near and far since in OpenGL we look down the negative z-axis.


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
            //do the perspective division
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

        var epsilon = 0.001;
        far = Math.min(-epsilon, far - epsilon);
        // don't let near get closer than 0.01 of far, and make sure near is
        // closer than far
        near = Math.min(0.01 * far, Math.max(far, near + epsilon));

        this._projection = mat4.perspective( 90, this._aspect, -near, -far );

With this code we have found the near and far plane based on the scenes bounding box and are now ready to create our projection matrix. 
The gl-matrix library can do the heavy lifting for us if we call `mat.perspective( verticalFieldOfView, aspectRatio, -near, -far )`, otherwise all we need to do is calculate top, bottom, left and right and plug it into the \f$projection \ matrix\f$ formula.
We negate near and far since in OpenGL we look down the negative z-axis.

As you might have noticed, the projection changes when either the bounding box or aspect ration changes, so we want to calculate a new version whenever that happens.
To do this we should place the above code into a seperate function and register _event_listeners_ that calls the method when needed.

So add the following code to the constructor of the `Tutorial6FPSViewer`:

        this._exposedModel.addLocalListener(this._boundingBoxKey, tinia.hitch(this, function(key, bb) {
            this.calculateProjectionMatrix();
            this.insertMatrices();
        }));

        this._exposedModel.addLocalListener(this._key, tinia.hitch(this, function(key, viewer) {
            var height = viewer.getElementValue("height");
            var width = viewer.getElementValue("width");
            if(height !== this._height || width !== this._width) {
                this._height = height;
                this._widht = width;
                this._aspect = width / height;
                this.calculateProjectionMatrix();
                this.insertMatrices();
            }
        }));


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

[^1] Sorry for using immediate mode, but it was the most compact way to do it