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
    console.log( "Constructing FPSViewer" );
    this._model = params.exposedModel;
    this._key = params.key;
    this._bbKey = params.boundingBoxKey;

    //...


    var viewDirection = vec3.createFrom( 0, 0, -1 );
    var viewRight = vec3.createFrom( 1, 0, 0 );
    var viewUp = vec3.createFrom( 0, 1, 0 );

    this._orientation = quat4.fromAxes( viewDirection, viewRight, viewUp );

    this._rotationStart = vec3.createFrom(0, 0, 0);

    this._cameraPosition = vec3.createFrom( 0, 0, 5 );
    this._lookAtPosition = vec3.createFrom( 0, 0, -1 );

    this._bbmin = vec3.create();
    this._bbmax = vec3.create();
    this.getBoundingBoxFromModel();

    this._modelView = mat4.identity( mat4.create() );
    this._projection = mat4.identity( mat4.create() );
    this._near  = -0.001;
    this._far = 1.0;
    this._fov = 90;

    var viewer = this._model.getElementValue(  this._key );
    this._width = viewer.getElementValue( "width" );
    this._height = viewer.getElementValue( "height" );
    this._aspect = this._width / this._height ;

    this.calculateMatrices();

}

FPSViewer.prototype = {

    keyPressEvent : function( event ) {
        console.log( event.key );
        var direction = vec3.create();
        var speed = 0.01;

        console.log( "camPos before move: " + this._cameraPosition );

        switch( event.key ){
        case 87 : this.moveForward( speed ); break;  //w, move forward in z direction
        case 83 : this.moveForward( -speed ); break;//s, back off mister
        case 65 : this.moveHorizontal( -speed ); break;//a, move left on x-axis
        case 68 : this.moveHorizontal( speed ); break;//moving right
        case 81 : this.moveVertical( speed ); break; //move up
        case 69 : this.moveVertical( -speed ); break; //move down
        }

        console.log( "camPos after  move: " + this._cameraPosition );

        // direction = quat4.multiplyVec3( this._orientation, direction );

        // console.log( "rotate direction " + direction );
        // console.log( "initial cameraPosition " + this._cameraPosition );

        // this._cameraPosition = vec3.add( this._cameraPosition, direction );
        // this._lookAtPosition = vec3.add( this._lookAtPosition, direction );

        // console.log( "updated cameraPosition " + this._cameraPosition );
        // console.log( "updated lookAtPosition " + this._lookAtPosition );
        console.log("modelview: " + this._modelView );
        this.calculateMatrices();

    },

    moveHorizontal: function( speed ) {
        console.log( "horizontal move:  "  + speed );

        var rightVec = vec3.createFrom( this._modelView[0], this._modelView[4], this._modelView[8] );

        console.log( "right vector:  " + rightVec );

        rightVec = vec3.scale( rightVec, speed );

        console.log( "scaled right vector:  " + rightVec );

        this._lookAtPosition = vec3.add( this._lookAtPosition, rightVec );

    },

    moveForward: function( speed ) {

        var forwardVec = vec3.createFrom( this._modelView[2], this._modelView[6], this._modelView[10] );

        console.log( "forward vector:  " + forwardVec );
        forwardVec = vec3.scale( forwardVec, speed );
        this._lookAtPosition = vec3.add( this._lookAtPosition, forwardVec );

    },

    moveVertical: function( speed ) {
        var upVec = vec3.createFrom( this._modelView[1], this._modelView[5], this._modelView[9] );
        console.log( "up vector " + upVec );
        upVec = vec3.scale( upVec, speed );
        this._lookAtPosition = vec3.add( this._lookAtPosition, upVec );

    },

    mousePressEvent : function( event ) {

        this._rotationStart = this.getPointOnUnitSphere( event.relativeX, event.relativeY );
    },

    mouseMoveEvent : function( event ) {

        var rotationEnd = this.getPointOnUnitSphere( event.relativeX, event.relativeY );

        var rotation = vec3.rotationTo( this._rotationStart, rotationEnd );

        if( rotation !== this._orientation ){

            this._orientation = quat4.multiply( rotation, this._orientation );
            this._rotationStart = rotationEnd;

        }

        this.calculateMatrices();
    },

    mouseReleaseEvent : function( event ) {

    },


    calculateMatrices : function( event ) {

        var negCamPos = vec3.negate( vec3.create( this._cameraPosition ) );
        this._modelView = mat4.identity( mat4.create() );
        //this._modelView = mat4.translate( this._modelView, negCamPos );
        this._modelView = mat4.multiply(  this._modelView, quat4.toMat4( this._orientation ) );
        //this._modelView = mat4.translate( this._modelView, this._cameraPosition);
        this._modelView = mat4.translate( this._modelView, this._lookAtPosition );

        var viewer = this._model.getElementValue( this._key );
        this._width = viewer.getElementValue( "width" );
        this._height = viewer.getElementValue( "height" );
        this._aspect = this._width / this._height ;
        //this._projection = mat4.perspective( 90.0, this._aspect, this._near, this._far );

        this._projection = this.createProjectionMatrix( this._fov, this._near);


        viewer.updateElement( "modelview", this._modelView );
        viewer.updateElement( "projection", this._projection );
        this._model.updateElement( this._key, viewer );
    },


    //Based on the "Projection Matrix found in "The Graphics Codex" by Morgan McGuire
    createProjectionMatrix: function(  ) {
        proj = mat4.identity( mat4.create( ) );

        var tanFov2 = Math.tan( this._fov / 2 );
        var k = 1 / tanFov2;

        proj[0] = this._aspect * k;
        proj[5] = k;
        proj[10] = ( this._near + this._far ) / (this._near - this._far );
        proj[11] = -( (2 * this._near * this._far) / (this._near - this._far ) );

        proj[14] = -1;
        proj[15] = 0;
        proj = mat4.transpose( proj );
        return proj;
    },

    //From DSRV.js
    getPointOnUnitSphere: function(x, y) {
        var nx = ((2.0 * x) / this._width - 1.0) * this._aspect;
        var ny = -((2.0 * y) / this._height - 1.0);
        var r2 = nx * nx + ny * ny;

        if (r2 < 1.0) {
            return vec3.create([nx, ny, Math.sqrt(1.0 - r2)]);
        }
        else {
            var r = 1.0 / Math.sqrt(r2);
            return vec3.create([r * nx, r * ny, 0.0]);
        }
    },


    updateBoundingBox : function(bb) {
        bb = bb.split(" ");
        this._bbmin = vec3.createFrom(bb[0] - 0.0, bb[1] - 0.0, bb[2] - 0.0);
        this._bbmax = vec3.createFrom(bb[3] - 0.0, bb[4] - 0.0, bb[5] - 0.0);
    },

    getBoundingBoxFromModel : function() {
        this.updateBoundingBox(this._model.getElementValue(this._bbKey));
    }

}

