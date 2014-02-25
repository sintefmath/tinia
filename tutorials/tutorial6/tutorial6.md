Tutorial 6: Creating your own viewer in Tinia, FPSviewer {#tut_tutorial6}
===

In this tutorial we build upon [Tutorial 3](@ref tut_tutorial3) and integrate our own viewer control into a Tinia Application.
Familiarity with basic OpenGL, C++ with object orientation and JavaScript is assumed, as well as a grasp on linear algebra.

We will first go over how you connect a new viewer to the Tinia Framework, how you select it from the C++ side, and what you need to do in Javascript before implementing the neccessary math.

Like the other tutorials, this program will run both as a desktop program and as a server/client program.
The program consists of four files: The Javascript file, the Job class definition and two main files. One main file will be created for the desktop program, and one for the web program.
These complete files are located under /tutorials/tutorial6/, and can also be viewed at the bottom of this tutorial.

\tableofcontents

Extending Tinias
====
Whilst Tinia comes with support for inspecting a scene/object through a trackball viewer, we realise this is not always the ideal solution and have taken steps to make it easy to create other types of viewers.
And so in this tutorial we will extend the viewer controls of Tinias renderwindow to support FPS-style navigation of the scene.

This then requires writing new _event handler_ methods that reacts to the user input, be it key presses, mouse movements etc., and translate them into camera movements.
We assume that you know how to set up a job in Tinia, if not please read the other tutorials, and will concentrate on the changes needed to implement the FPS camera.

To keep the GUI as responsive as possible for the end user, these methods should be run as "close" to the user as possible.
In a web-application, this requirement mandates that the user input is processed client-side in the web browser and hence have to be implemented in Javascript.
And since Tinia supports both web- and desktop-applications, we have included a Javascript engine in Tinia so the Javascript viewer also works for desktop-applications.


Mapping between Tinia and Javascript code
===
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
`this.calculateProjectionMatrix()` also creates a new \f$modelView\f$ matrix, since it uses this when transforming the boundingbox into world space. If it didn't do this we could get problems with parts of the scene getting culled by the near/far plane.
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
    


Receiving user input
===
The previous section shows how we can listen for changes to the exposed model and store data into Tinia that will be used for rendering, both of which are needed to implement a viewer.
However, we are missing the most important part, how to receive the user input of which we will create our matrices.

This is very similar to how we listened for changes to the exposed model, except the listeners are already defined and all we have to do is implement the functions.
For mouse input it is `mousePressEvent`, `mouseMoveEvent` and `mouseReleaseEvent`; for touch it is `touchStartEvent`, `touchMoveEvent` and `touchEndEvent`; and for keyboard it is simply `keyPressEvent`.

We will only be implementing the mouse and keyboard functions for our FPSviewer, with touch being left as an excercise for the reader. (You can find an implementation in DSRV.js)

For our viewer we will only allow the user to move in/out of the scene in the direction the camera is looking. So our keyboard event handling is quite simply:

    keyPressEvent: function ( event ) {
        var speed = 0.1;
        switch( event.key ){
            case 87 : this.m_moveForward = speed; break; //w, move camera in z direction
            case 83 : this.m_moveForward = -speed; break; //s, move camera in reverse z direction
        }
        //need to update matrices and store them
        this.calculateProjectionMatrix();
        this.insertMatrices();
    }

Our mouse handling is somewhat more involved, but still not very complex. To make our viewer more suitable for use on both desktop and browser applications, we only want to rotate when a mouse button is pressed.
This means we will need to have two different states, one for rotating and one for no action with the latter being set when the mouse button is no longer pressed:

    //from the constructor
    this.NONE = -1;
    this.m_state = this.NONE;
    this.ROTATE = 1;
    ...

    mouseReleaseEvent: function( event ){
        this.m_state = this.NONE;  
    },

When we start rotating we need to likewise set the state to `ROTATE`, and store the mouse position for where the rotation started:

    mousePressEvent: function( event ) {
        this.m_rotationStart = vec2.createFrom( event.relativeX, event.relativeY );
        this.m_state = this.ROTATE;
    },

Lastly we need to handle the rotations. We want to allow the user to turn around freely, but to make it a bit easier to control we want to limit the user to looking up, forwards and down without being able to do backflips. 
So we lock the rotation around the X-axis to +- 89 degrees, and let the rotation around the Y-axis spin as many times as it wants (though keeping in the range 0 - 369 degrees). 
We keep the rotations separate, with movement in the `relativeX` translating to rotations around the the Y-axis, and `relativeY` around the X-axis.

    mouseMoveEvent: function( event ) {
        if(this.m_state == this.ROTATE){
            var rotationEnd = vec2.createFrom( event.relativeX, event.relativeY );

            this.m_leftRightRotation += rotationEnd[0] - this.m_rotationStart[0];

            if( this.m_leftRightRotation < 0.0){
                this.m_leftRightRotation += 360.0;
            }else if( this.m_leftRightRotation > 360.0){
                this.m_leftRightRotation -= 360.0;
            }
            
            this.m_upDownRotation = this.m_upDownRotation + (rotationEnd[1] - this.m_rotationStart[1]);
            if(this.m_upDownRotation < -89.0 ){
                this.m_upDownRotation = -89.0;
            }else if(this.m_upDownRotation > 89.0){
                this.m_upDownRotation = 89.0;
            }
            
            this.m_rotationStart = rotationEnd;
            
            this.calculateProjectionMatrix();
            this.insertMatrices();
        }
     }

Now that we have gotten the user input, it is time to calculate the matrices that will actually move the objects on the screen.

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

     calculateProjectionMatrix: function () {
        this.calculateModelView();

        //taken from DSRV.js
        // --- set up projection matrix
        // the eight corners of the bounding box
        var corners = [[this.m_bbMin[0], this.m_bbMin[1], this.m_bbMin[2], 1.0],
                       [this.m_bbMin[0], this.m_bbMin[1], this.m_bbMax[2], 1.0],
                       [this.m_bbMin[0], this.m_bbMax[1], this.m_bbMin[2], 1.0],
                       [this.m_bbMin[0], this.m_bbMax[1], this.m_bbMax[2], 1.0],
                       [this.m_bbMax[0], this.m_bbMin[1], this.m_bbMin[2], 1.0],
                       [this.m_bbMax[0], this.m_bbMin[1], this.m_bbMax[2], 1.0],
                       [this.m_bbMax[0], this.m_bbMax[1], this.m_bbMin[2], 1.0],
                       [this.m_bbMax[0], this.m_bbMax[1], this.m_bbMax[2], 1.0]];
        // apply the modelView matrix to the eight corners to get the minimum
        // and maximum z in the camera's local coordinate system.
        var near, far;
        for (i in corners) {
            var c = corners[i];
            var p = mat4.multiplyVec4(this.m_modelView, c);
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
        this.m_projection = mat4.perspective( 90.0, this.m_aspect, -near, -far );
    },


Createing the model view matrix
---
Now the only thing we are missing is `this.calculateModelView()`, where we actually do our rotations and translations.
There are many ways this could be done, and here we have chosen a bit verbose version to make it easier to follow.

    calculateModelView: function () {
        var udAngle = (this.m_upDownRotation * Math.PI) / 180.0;
        var lrAngle = (this.m_leftRightRotation * Math.PI) / 180.0;
        var rotX = mat4.rotateX( mat4.identity(), udAngle );
        var rotY = mat4.rotateY( mat4.identity(), lrAngle );

        var movement = vec4.createFrom(0.0, 0.0, -1.0, 0.0);
        movement = mat4.multiplyVec4( rotX, movement );
        movement = mat4.multiplyVec4( rotY, movement );
        doMove = vec4.scale( movement, this.m_moveForward, vec4.create() );

        this.m_cameraPosition = vec4.add( this.m_cameraPosition, doMove, vec4.create() );

        var transMat = mat4.translate( mat4.identity(), vec4.negate(this.m_cameraPosition, vec4.create() ) );
        var rotated = mat4.multiply(rotX, rotY);
        var rotxyTranspose = mat4.transpose(rotated);
        this.m_modelView = mat4.multiply( rotxyTranspose, transMat );

        this.m_moveForward = 0;
    }


Inspecting the results
===
With all the pieces in place, you should be able to load up the viewer script and move around in your favourite scene with an FPS camera. 
Should you be lacking a proper scene to interact with, and are getting confused trying to navigate a scene with the single red triangle we have used in other tutorials, you can try the following code snippet.
It renders a simple box with differently coloured sides, and should give you enough of a feeling for if the camera works as expected or not.
(Sorry for using immediate mode, but it was the most compact way to do it)
\snippet tutorial6_job.hpp drawColoredBox 

Add this snippet to your overloaded [renderFrame](@ref tinia::jobcontroller::OpenGLJob::renderFrame) function and you should get a nice box on the screen. 
If it looks a bit strange, make sure that the depth test is enabled and the depth buffer properly cleared.
(This is done using `glEnable(GL_DEPTH_TEST);` and `glClear( GL_DEPTH_BUFFER_BIT );`)
 

If you want look at the whole code for the tutorial, enjoy :)

The full Job file
---
All changes in this tutorial have been done in the Job file of the tutorial:
\include tutorial6_job.hpp

The full Javascript file
---
All the Javascript code in this tutorial can be found in the .js file of the tutorial
\include tutorial6.js

