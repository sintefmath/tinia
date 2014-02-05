/* Copyright STIFTELSEN SINTEF 2012
 *
 * This file is part of the Tinia Framework.
 *
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */
function tutorial6( params ) {
    this.m_model = params.exposedModel;
    this.m_key = params.key;
    this.m_bboxKey = params.boundingBoxKey;

    this.m_bbMin = vec3.create();
    this.m_bbMax = vec3.create();
    this.m_modelView = mat4.identity( mat4.create() );
    this.m_projection = mat4.identity( mat4.create() );

    var viewer = this.m_model.getElementValue( this.m_key );
    this.m_width = viewer.getElementValue( "width" );
    this.m_height = viewer.getElementValue( "height" );
    this.m_aspect = this.m_width / this.height;        

    this.m_leftRightRotation = 0.0;
    this.m_upDownRotation = 0.0;
    this.m_rotationStart = vec2.create();
    this.m_moveSpeed = 1.0;
    this.m_forward = vec3.create(0.0, 0.0, -4.0);
    this.m_up = vec3.create( 0.0, 1.0, 0.0);
    this.m_right = vec3.create( 1.0, 0.0, 0.0);

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

    updateBoundingBox : function(bb) {
        bb = bb.split(" ");
        this.m_bbmin = vec3.createFrom(bb[0] - 0.0, bb[1] - 0.0, bb[2] - 0.0);
        this.m_bbmax = vec3.createFrom(bb[3] - 0.0, bb[4] - 0.0, bb[5] - 0.0);
    },
    
    calculateProjectionMatrix: function () {
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
        var epsilon = 0.001;
        far = Math.min(-epsilon, far - epsilon);
        // don't let near get closer than 0.01 of far, and make sure near is
        // closer than far
        near = Math.min(0.01 * far, Math.max(far, near + epsilon));
        var top = near * Math.tan( 45 * (Math.PI / 360.0) );
        var right = top * this.m_aspect;
        this.m_projection = mat4.perspective( 90.0, this.m_aspect, -near, -far );
//        this.m_projection = mat4.create(mat4.identity());
//        this.m_projection[0] = (2 * near) / (right + right);
//        this.m_projection[1] = 0;
//        this.m_projection[2] = 0;
//        this.m_projection[3] = 0;
//        this.m_projection[4] = 0;
//        this.m_projection[5] = (2 * near)/ (top + top);
// this.m_projection[6] = 0;
// this.m_projection[7] = 0;
// this.m_projection[8] = (right + (-right)/(right - (-right));
// this.m_projection[9] = (top + (-top))/(top - (-top));
// this.m_projection[10] = -(far+near)/(far-near);
// this.m_projection[11] = -1;
// this.m_projection[12] = 0;
// this.m_projection[13] = 0;
// this.m_projection[14] = -(far*near*2)/(far-near);
// this.m_projection[15] = 0;
//
        //mat4.frustum( -right, right+epsilon, -top, top+epsilon, near, far, this.m_projection);
        //        this.m_projection = mat4.create(mat4.identity());

    },

    calculateModelView: function () {
        var udAngle = this.m_upDownRotation * (180.0 / Math.PI );
        var qRot = quat4.fromAngleAxis( udAngle, this.m_right, quat4.create()); 
        var lrAngle = this.m_leftRightRotation * (180.0 / Math.PI );
        qRot *= quat4.fromAngleAxis( lrAngle, this.m_up, quat4.create()); 
        var movement = quat4.multiplyVec3(qRot, this.m_forward); // movement
        this.m_modelView = mat4.fromRotationTranslation(qRot, movement, this.m_modelView); //get final modelView
        this.m_modelView = mat4.create(mat4.identity());
    },

    mousePressEvent: function( event ) {
        this.m_rotationStart = vec2.create( event.relativeX, event.relativeY );
    },

    mouseMoveEvent: function( event ) {
        var rotationEnd = vec2.create( event.relativeX, event.relativeY );

        this.m_leftRightRotation += rotationEnd[0] - this.m_rotationStart[0];

       if( this.m_leftRightRotation < 0.0){
           this.m_leftRightRotation += 360.0;
       }else if( this.m_leftRightRotation > 360.0){
           this.m_leftRightRotation -= 360.0;
       }
        
       this.m_upDownRotation = this.m_upDownRotation - (rotationEnd[1] - this.m_rotationStart[1]);
       if(this.m_upDownRotation < -89.0 ){
           this.m_upDownRotation = -89.0;
       }else if(this.m_upDownRotation > 89.0){
           this.m_upDownRotation = 89.0;
       }
       
       this.m_rotationStart = rotationEnd;
       
       this.calculateModelView();
    },
                                  
    keyPressEvent: function ( event ) {
        var speed = 1;
        switch( event.key ){
            case 87 : this.moveForward( speed ); break; //w, move camera in z direction
            case 83 : this.moveForward( -speed); break; //s, move camera in reverse z direction
        }
    },

    moveForward: function ( speed ) {
        this.m_forward += vec3.create( 0.0, 0.0, speed*this.m_moveSpeed );
    }
}

