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
//        console.log(" bbMin.x: " + this.m_bbMin[0] + " bbMin.y: " + this.m_bbMin[1] + " bbMin.z: " + this.m_bbMin[2] );
//        console.log(" bbMax.x: " + this.m_bbMax[0] + " bbMax.y: " + this.m_bbMax[1] + " bbMax.z: " + this.m_bbMax[2] ); 

    this.m_modelView = mat4.identity( mat4.create() );
    this.m_projection = mat4.identity( mat4.create() );

    var viewer = this.m_model.getElementValue( this.m_key );
    this.m_width = (viewer.getElementValue( "width" ) - 0.0);
    this.m_height = (viewer.getElementValue( "height" ) - 0.0);
    this.m_aspect = (this.m_width - 0.0) / (this.m_height-0.0);

    console.log("w: " + this.m_width + ", h: " + this.m_height + ", aspect: " + this.m_aspect);

    this.m_leftRightRotation = 0.0;
    this.m_upDownRotation = 0.0;
    this.m_rotationStart = vec2.create();
    this.m_moveSpeed = 1.0;
    this.m_forward = vec3.createFrom(0.0, 0.0, -0.001);
    this.m_up = vec3.createFrom( 0.0, 1.0, 0.0);
    this.m_right = vec3.createFrom( 1.0, 0.0, 0.0);

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
    this.calculateProjectionMatrix();
    this.calculateModelView();
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
        // --- set up projection matrix
        console.log("Calculating projection matrix");
        console.log(" bbMin.x: " + this.m_bbMin[0] + " bbMin.y: " + this.m_bbMin[1] + " bbMin.z: " + this.m_bbMin[2] );
        console.log(" bbMax.x: " + this.m_bbMax[0] + " bbMax.y: " + this.m_bbMax[1] + " bbMax.z: " + this.m_bbMax[2] ); 

        //var first = true;
        //if( this.m_modelView[0] != this.m_modelView[0] && first ){
        //    first = false;
        //  console.log("Modelview has NaNs, recalculating it!");
        this.calculateModelView();
        //}
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
//this.m_projection = mat4.create();
//console.log("aspect: " + this.m_aspect);
//console.log("right: " + right + ", left: " + left);
//console.log("top: " + top + ", left: " + bottom);
//this.m_projection[0] = (2 * near) / (right - left);
//this.m_projection[1] = 0;
//this.m_projection[2] = 0;
//this.m_projection[3] = 0;
//this.m_projection[4] = 0;
//this.m_projection[5] = (2 * near)/ (top - bottom);
//this.m_projection[6] = 0;
//this.m_projection[7] = 0;
//this.m_projection[8] = (right + left)/(right - left);
//this.m_projection[9] = (top + bottom)/(top - bottom);
//this.m_projection[10] = -(far+near)/(far-near);
//this.m_projection[11] = -1;
//this.m_projection[12] = 0;
//this.m_projection[13] = 0;
//this.m_projection[14] = -(far*near*2)/(far-near);
//this.m_projection[15] = 0;
//
        //mat4.frustum( -right, right+epsilon, -top, top+epsilon, near, far, this.m_projection);
        //        this.m_projection = mat4.create(mat4.identity());
        console.log("projection: "+ this.m_projection[0] + ", " + this.m_projection[1] + ", " + this.m_projection[2] + ", " + this.m_projection[3] + ", " + this.m_projection[4] + ", " + this.m_projection[5] + ", " + this.m_projection[6] + ", " + this.m_projection[7] + ", " + this.m_projection[8] + ", " + this.m_projection[9] + ", " + this.m_projection[10] + ", " + this.m_projection[11] + ", " + this.m_projection[12] + ", " + this.m_projection[13] + ", " + this.m_projection[14] + ", " + this.m_projection[15]);
        console.log("Done calculating projection");
    },

    calculateModelView: function () {
        console.log("Calculating modelView");
        
        var udAngle = (this.m_upDownRotation * Math.PI) / 180.0;
        console.log("udr: " + this.m_upDownRotation + ", udAngle: " + udAngle);
        var qRot = quat4.fromAngleAxis( udAngle, this.m_right, quat4.create());
        console.log("qRot: " + qRot[0] + ", " + qRot[1] + ", " + qRot[2] + ", " + qRot[3]);
        var lrAngle = (this.m_leftRightRotation * Math.PI) / 180.0;
        console.log("lrr: " + this.m_leftRightRotation + ", lrAngle: " + lrAngle);
        var qRot2 = quat4.fromAngleAxis( lrAngle, this.m_up, quat4.create()); 
        console.log("qRot2: " + qRot2[0] + ", " + qRot2[1] + ", " + qRot2[2] + ", " + qRot2[3]);

        console.log("this.m_forward: " + this.m_forward[0] + ", " + this.m_forward[1] + ", " + this.m_forward[2]);
        qRot = quat4.multiply(qRot, qRot2);
        console.log("qRot: " + qRot[0] + ", " + qRot[1] + ", " + qRot[2] + ", " + qRot[3]);
        var movement = quat4.multiplyVec3(qRot, this.m_forward); // movement
        console.log("mvmnt.x: " + movement[0] + " mvmnt.y: " + movement[1] + " mvmnt.z: " + movement[2] );
        this.m_modelView = mat4.fromRotationTranslation(qRot, movement, this.m_modelView); //get final modelView
//        this.m_modelView = mat4.create(mat4.identity());
        console.log("modelView: "+ this.m_modelView[0] + ", " + this.m_modelView[1] + ", " + this.m_modelView[2] + ", " + this.m_modelView[3] + ", " + this.m_modelView[4] + ", " + this.m_modelView[5] + ", " + this.m_modelView[6] + ", " + this.m_modelView[7] + ", " + this.m_modelView[8] + ", " + this.m_modelView[9] + ", " + this.m_modelView[10] + ", " + this.m_modelView[11] + ", " + this.m_modelView[12] + ", " + this.m_modelView[13] + ", " + this.m_modelView[14] + ", " + this.m_modelView[15]);
        console.log("Done calculating modelView");
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
            
            this.calculateModelView();
            this.insertMatrices();
        }
    },

    mouseReleaseEvent: function( event ){
        this.m_state = this.NONE;  
    },
                                  
    keyPressEvent: function ( event ) {
        var speed = 1;
        switch( event.key ){
            case 87 : this.moveForward( speed ); break; //w, move camera in z direction
            case 83 : this.moveForward( -speed); break; //s, move camera in reverse z direction
        }
    },

    moveForward: function ( speed ) {
        this.m_forward += vec3.createFrom( 0.0, 0.0, speed*this.m_moveSpeed );
    }
}

