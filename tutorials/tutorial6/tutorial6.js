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

function FPSViewer( params ) {
    this.m_model = params.exposedModel;
    this.m_key = params.key;
    this.m_bboxKey = params.boundingBoxKey;

    this.m_modelView = mat4.identity( mat4.create() );
    this.m_projection = mat4.identity( mat4.create() );

    this.m_bbMin = vec3.create();
    this.m_bbMax = vec3.create();

    var viewer = this.m_model.getElementValue( this.m_key );
    this.m_width = viewer.getElementValue( "width" );
    this.m_height = viewer.getElementValue( "height" );
    this.m_aspect = this.m_width / this.height;        

    this.m_leftRightRotation = 0;
    this.m_upDownRotation = 0;
    this.m_rotationStart = vec2.create();
    this.m_moveSpeed = 0;
    this.m_foward = vec3.create(0, 0, 1);

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
}

FPSViewer.prototype = {
    insertMatrices: function () {
        var viewer = this.m_exposedModel.getElementValue(this.m_key);
        viewer.updateElement("modelview", this.m_modelView);
        viewer.updateElement("projection", this.m_projection);
        this.m_exposedModel.updateElement(this.m_key, viewer);
    },

    updateBoundingBox : function(bb) {
        bb = bb.split(" ");
        this._bbmin = vec3.createFrom(bb[0] - 0.0, bb[1] - 0.0, bb[2] - 0.0);
        this._bbmax = vec3.createFrom(bb[3] - 0.0, bb[4] - 0.0, bb[5] - 0.0);
    },
    
    calculateProjectionMatrix: function () {
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
    },

    calculateModelView: function () {
        var udAngle = this.m_upDownRotation * (180 / Math.PI );
        var qRot = quat4.fromAngleAxis( udAngle, this.m_right); //up down rotates around x axis
        var lrAngle = this.m_leftRightRotation * (180 / Math.PI );
        qRot *= quat4.fromAngleAxis( lrAngle, this.m_up); //right left rotates around y axis
        var movement = quat4.multiplyVec3 = function (qRot, this.m_foward); // movement
        this.m_modelview = mat4.fromRotationTranslation = function (qRot, movement, this.m_modelview); //get final modelview
    },

    mousePressEvent: function( event ) {
        this.m_rotationStart = vec2.create( event.relativeX, event.relativeY );
    },

    mouseMoveEvent: function( event ) {
        var rotationEnd = vec2.create( event.relativeX, event.relativeY );


        this.m_leftRightRotation += rotationEnd[0] - this.m_rotationStart[0];

        if( this.m_leftRightRotation < 0){
            this.m_leftRightRotation += 360;
        }else if( this.m_leftRightRotation > 360){
            this.m_leftRightRotation -= 360;
        }

        this.m_upDownRotation += rotationEnd[1] - this.m_rotationStart.[1];
        if(this.m_upDownRotation < -89 ){
            this.m_upDownRotation = -89;
        }else if(this.m_upDownRotation > 89){
            this.m_upDownRotation = 89;
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
        this.m_forward += vec3.create( 0, 0, speed*this.m_moveSpeed );
    }
}

