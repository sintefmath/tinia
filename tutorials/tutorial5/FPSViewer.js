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
    this._near  = 0.0001;
    this._far = 1.0;

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
        var speed = 1;

	console.log( "camPos before move: " + this._cameraPosition );
	
        switch( event.key ){
        case 87 : direction[2] = moveForward( speed ); break;  //w, move forward in z direction
        case 83 : direction[2] = moveForward( -speed ); break;//s, back off mister
        case 65 : direction[0] = moveHorizontal( speed ); break;//a, move left on x-axis
        case 68 : direction[0] = moveHorizontal( -speed ); break;//moving right
        }

	console.log( "camPos after  move: " + this._cameraPosition );
        // direction = quat4.multiplyVec3( this._orientation, direction );

        // console.log( "rotate direction " + direction );
        // console.log( "initial cameraPosition " + this._cameraPosition );

        // this._cameraPosition = vec3.add( this._cameraPosition, direction );
        // this._lookAtPosition = vec3.add( this._lookAtPosition, direction );

        // console.log( "updated cameraPosition " + this._cameraPosition );
        // console.log( "updated lookAtPosition " + this._lookAtPosition );

        this.calculateMatrices();

    },

    moveHorizontal: function( speed ) {
	console.log( "horizontal move:  "  + speed );
	
	var rightVec = vec3.create( this._modelView[0], this._modelView[4], this._modelView[8] );

	console.log( "right vector:  " + rightVec );
	
	rightVec = vec3.scale( rightVec, speed );

	console.log( "scaled right vector:  " + rightVec );
	
	this._cameraPosition = vec3.multiply( this._cameraPosition, rightVec );

    },

    moveForward: function( speed ) {
	var forwardVec = vec3.create( this._modelView[1], this._modelView[5], this._modelView[9] );
	forwardVec = vec3.scale( forwardVec, speed );
	this._cameraPosition = vec3.multiply( this._cameraPosition, forwardVec );

    },

    moveVertical: function( speed ) {
	var upVec = vec3.create( this._modelView[1], this._modelView[5], this._modelView[9] );
	upVec = vec3.scale( upVec, speed );
	this._cameraPosition = vec3.multiply( this._cameraPosition, upVec );

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
        console.log( "cameraPos: " + this._cameraPosition );
        var negCamPos = vec3.negate( vec3.create( this._cameraPosition ) );
        this._modelView = mat4.identity( mat4.create() );
        this._modelView = mat4.translate( this._modelView, negCamPos );
        this._modelView = mat4.multiply(  this._modelView, quat4.toMat4( this._orientation ) );
        this._modelView = mat4.translate( this._modelView, this._cameraPosition);
//        this._modelView = mat4.translate( this._modelView, this._lookAtPosition );

        var viewer = this._model.getElementValue( this._key );
        this._width = viewer.getElementValue( "width" );
	this._height = viewer.getElementValue( "height" );
	this._aspect = this._width / this._height ;
        this._projection = mat4.perspective( 90.0, this._aspect, this._near, this._far );



        console.log( this._cameraPosition );
        console.log( "modelview " + this._modelView );
        console.log( "projection " + this._projection );

        viewer.updateElement( "modelview", this._modelView );
        viewer.updateElement( "projection", this._projection );
        this._model.updateElement( this._key, viewer );
    },


//Based on the "Infinite Projection Matrix found in "The Graphics Codex" by Morgan McGuire
    createInfiniteProjectionMatrix: function( fov, near ) {
	proj = mat4.create( mat4.identity() );

	var tanFov2 = Math.tan( fov / 2 );
	var k = 1 / tanFov2;

	proj[0] = this._aspect * k;
	proj[2] = k / ( near * this._width );

	proj[5] = k;
	proj[6] = k / (near * this._height );

	proj[10] = -1;
	proj[11] = 2 * near;
	
	proj[14] = -1;
	proj[15] = 0;

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

