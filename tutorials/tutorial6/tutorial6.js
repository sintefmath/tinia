function tutorial6( params ) {
    this.m_model = params.exposedModel;
    this.m_key = params.key;
    this.m_bboxKey = params.boundingBoxKey;

    this.NONE = -1;
    this.m_state = this.NONE;
    this.ROTATE = 1;

    this.m_bbMin = vec3.create();
    this.m_bbMax = vec3.create();
    this.getBoundingBoxFromModel();


    this.m_modelView = mat4.identity( mat4.create() );
    this.m_projection = mat4.identity( mat4.create() );

    var viewer = this.m_model.getElementValue( this.m_key );
    this.m_width = (viewer.getElementValue( "width" ) - 0.0);
    this.m_height = (viewer.getElementValue( "height" ) - 0.0);
    this.m_aspect = (this.m_width - 0.0) / (this.m_height-0.0);

    this.m_cameraPosition = vec4.createFrom( 0.0, 0.0, 0.0, 1.0 );
    this.m_leftRightRotation = 0.0;
    this.m_upDownRotation = 0.0;
    this.m_rotationStart = vec2.create();
    this.m_moveSpeed = 1.0;
    this.m_moveForward = 0.0;
    this.m_forward = vec4.createFrom(0.0, 0.0, -1.0, 0.0);
    this.m_up = vec4.createFrom( 0.0, 1.0, 0.0, 0.0);
    this.m_right = vec4.createFrom( 1.0, 0.0, 0.0, 0.0);

    this.m_model.addLocalListener(this.m_boundingBoxKey, tinia.hitch(this, function(key, bb) {
        this.updateBoundingBox(bb);
        this.calculateProjectionMatrix();
        this.insertMatrices();

    }));

    this.m_model.addLocalListener(this.m_key, tinia.hitch(this, function(key, viewer) {
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
    this.calculateModelView();
    this.calculateProjectionMatrix();
    this.insertMatrices();
}

tutorial6.prototype = {
    insertMatrices: function () {
        var viewer = this.m_model.getElementValue(this.m_key);
        viewer.updateElement("modelview", this.m_modelView);
        viewer.updateElement("projection", this.m_projection);
        this.m_model.updateElement(this.m_key, viewer);
    },

    getBoundingBoxFromModel: function () {
        this.updateBoundingBox( this.m_model.getElementValue( this.m_bboxKey ) );
    },
    updateBoundingBox : function(bb) {
        bb = bb.split(" ");
        this.m_bbMin = vec3.createFrom(bb[0] - 0.0, bb[1] - 0.0, bb[2] - 0.0);
        this.m_bbMax = vec3.createFrom(bb[3] - 0.0, bb[4] - 0.0, bb[5] - 0.0);
    },
    
    calculateProjectionMatrix: function () {
        this.calculateModelView();
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
        console.log( "far ", far);
        console.log("near: ", near);
        var epsilon = 0.001;
        far = Math.min(-epsilon, far - epsilon);
        // don't let near get closer than 0.01 of far, and make sure near is
        // closer than far
        near = Math.min(0.01 * far, Math.max(far, near + epsilon));
        var top = near * Math.tan( 45 * (Math.PI / 360.0) );
        var right = top * this.m_aspect;
        var left = epsilon - right;
        var bottom = epsilon - top;
        this.m_projection = mat4.perspective( 90.0, this.m_aspect, -near, -far );
        this.m_moveSpeed = far - near;

    },

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
    },

    mousePressEvent: function( event ) {
        this.m_rotationStart = vec2.createFrom( event.relativeX, event.relativeY );
        this.m_state = this.ROTATE;
    },

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
    },

    mouseReleaseEvent: function( event ){
        this.m_state = this.NONE;  
    },
                                  
    keyPressEvent: function ( event ) {
        var speed = 0.1;
        switch( event.key ){
            case 87 : this.m_moveForward = speed; break; //w, move camera in z direction
            case 83 : this.m_moveForward = -speed; break; //s, move camera in reverse z direction
        }

        this.calculateProjectionMatrix();
        this.insertMatrices();
    }
}

