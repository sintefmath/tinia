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
       // console.log("projection: "+ this.m_projection[0] + ", " + this.m_projection[1] + ", " + this.m_projection[2] + ", " + this.m_projection[3] + ", " + this.m_projection[4] + ", " + this.m_projection[5] + ", " + this.m_projection[6] + ", " + this.m_projection[7] + ", " + this.m_projection[8] + ", " + this.m_projection[9] + ", " + this.m_projection[10] + ", " + this.m_projection[11] + ", " + this.m_projection[12] + ", " + this.m_projection[13] + ", " + this.m_projection[14] + ", " + this.m_projection[15]);
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

        console.log("this.m_cameraPosition before: " + this.m_cameraPosition[0] + ", " + this.m_cameraPosition[1] + ", " + this.m_cameraPosition[2] + ", " + this.m_cameraPosition[3]);
        console.log("eyespace doMove: " + doMove[0] + ", "+ doMove[1] + ", "+ doMove[2] );
        this.m_cameraPosition = vec4.add( this.m_cameraPosition, doMove, vec4.create() );
        console.log("this.m_cameraPosition after: " + this.m_cameraPosition[0] + ", " + this.m_cameraPosition[1] + ", " + this.m_cameraPosition[2] + ", " + this.m_cameraPosition[3]);

        var transMat = mat4.translate( mat4.identity(), vec4.negate(this.m_cameraPosition, vec4.create() ) );
        var rotated = mat4.multiply(rotX, rotY);
        var rotxyTranspose = mat4.transpose(rotated);
        this.m_modelView = mat4.multiply( rotxyTranspose, transMat );

        var c = vec4.createFrom( this.m_cameraPosition[0], this.m_cameraPosition[1], this.m_cameraPosition[2], 1.0);
        var camPos = mat4.multiplyVec4(this.m_modelView, c, vec4.create());

        console.log("eyespace camPos: " + camPos[0] + ", "+ camPos[1] + ", "+ camPos[2] + ", "+ camPos[3]);
        var tempy = vec4.createFrom( doMove[0], doMove[1], doMove[2], 0.0);
        tempy = vec4.add(tempy, this.m_cameraPosition, vec4.create() );
        console.log("tempy: " + tempy[0] + ", "+ tempy[1] + ", "+ tempy[2] + ", "+ tempy[3]);
        tempy = mat4.multiplyVec4( this.m_modelView, tempy);
        console.log("eyespace tempy: " + tempy[0] + ", "+ tempy[1] + ", "+ tempy[2] + ", "+ tempy[3]);

//        var qRot = quat4.fromAngleAxis( udAngle, this.m_right, quat4.create());
//        var qRot2 = quat4.fromAngleAxis( lrAngle, this.m_up, quat4.create()); 
//        var qRot3 = quat4.multiply(qRot, qRot2);
//        var negTransMat = mat4.translate( mat4.identity(), vec3.negate(this.m_cameraPosition, vec3.create()) );
//        console.log("qRot after rotation: " + qRot[0] + ", " + qRot[1] + ", " + qRot[2] + ", " + qRot[3]);
//        console.log("lr: " + this.m_leftRightRotation+ " lrA: " + lrAngle + " ud: " + this.m_upDownRotation+ " udAngle: " + udAngle);
//        console.log("mov: " + this.m_moveForward);
//        var rotMat = quat4.toMat4( qRot3 );
//        var movement = quat4.multiplyVec3(qRot, this.m_forward, vec3.create() ); // movement
//        movement = vec3.normalize(movement);
//        movement = quat4.multiplyVec3(qRot2, this.m_forward, movement ); // movement
//        movement = vec3.normalize(movement);
        //var movement = this.m_forward;
//        console.log("mvmnt.x: " + movement[0] + " mvmnt.y: " + movement[1] + " mvmnt.z: " + movement[2] );
//        this.m_modelView = mat4.fromRotationTranslation(qRot, this.m_cameraPosition, this.m_modelView); //get final modelView
        
//        var lookAt = vec3.add( this.m_cameraPosition, vec3.scale( movement, 4.0), vec3.create() );
//        console.log("lookAt after rotation: " + lookAt[0] + ", " + lookAt[1] + ", " + lookAt[2]);
//        this.m_modelView = mat4.lookAt( this.m_cameraPosition, lookAt, this.m_up, this.m_modelView);

        console.log("modelView: "+ this.m_modelView[0] + ", " + this.m_modelView[1] + ", " + this.m_modelView[2] + ", " + this.m_modelView[3] + ", " + this.m_modelView[4] + ", " + this.m_modelView[5] + ", " + this.m_modelView[6] + ", " + this.m_modelView[7] + ", " + this.m_modelView[8] + ", " + this.m_modelView[9] + ", " + this.m_modelView[10] + ", " + this.m_modelView[11] + ", " + this.m_modelView[12] + ", " + this.m_modelView[13] + ", " + this.m_modelView[14] + ", " + this.m_modelView[15]);
//        console.log("Done calculating modelView");
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
            
            this.calculateModelView();
            this.insertMatrices();
        }
    },

    mouseReleaseEvent: function( event ){
        this.m_state = this.NONE;  
    },
                                  
    keyPressEvent: function ( event ) {
        var speed = 0.1;
        switch( event.key ){
            case 87 : this.m_moveForward = -speed; break; //w, move camera in z direction
            case 83 : this.m_moveForward = speed; break; //s, move camera in reverse z direction
        }
        this.calculateModelView();
        this.insertMatrices();


    },

//    moveForward: function ( speed ) {
//        var udAngle = (this.m_upDownRotation * Math.PI) / 180.0;
//        var qRot = quat4.fromAngleAxis( udAngle, this.m_right, quat4.create());
//        var lrAngle = (this.m_leftRightRotation * Math.PI) / 180.0;
//        var qRot2 = quat4.fromAngleAxis( lrAngle, this.m_up, quat4.create()); 
//        qRot = quat4.multiply(qRot, qRot2);
//
//        console.log("forward before update: " + this.m_forward[0] + ", " + this.m_forward[1] + ", " + this.m_forward[2]);
//        var tempy = quat4.multiplyVec3( qRot, this.m_forward, vec3.create());
//        tempy = vec3.normalize(tempy);
//        console.log("tempy after rotation: " + tempy[0] + ", " + tempy[1] + ", " + tempy[2]);
//        console.log("speed: " + speed*this.m_moveSpeed);
//        var moved = vec3.scale( tempy, speed*this.m_moveSpeed );
//        console.log("moved after scale: " + moved[0] + ", " + moved[1] + ", " + moved[2]);
//        this.m_cameraPosition = vec3.add(this.m_cameraPosition, moved);
//
//        var lookAt = vec3.add( this.m_cameraPosition, vec3.scale( moved, 4.0), vec3.create() );
//        this.m_modelView = mat4.lookAt( this.m_cameraPosition, lookAt, this.m_up, this.m_modelView);
//
//        console.log("updated camera position: " + this.m_cameraPosition[0] + ", " + this.m_cameraPosition[1] + ", " + this.m_cameraPosition[2]);
//
////        this.m_modelView = mat4.fromRotationTranslation(qRot, this.m_cameraPosition, this.m_modelView); //get final modelView
//        this.insertMatrices();
//
//    }
}

