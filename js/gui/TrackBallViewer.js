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

/* 
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

dojo.require("3rdparty.glMatrix");

function axisAngle(axis, angle) {
    var q = quat4.create();
    var s = Math.sin(0.5 * angle);
    var c = Math.cos(0.5 * angle);

    q[0] = s * axis[0];
    q[1] = s * axis[1];
    q[2] = s * axis[2];
    q[3] = -c;

    return q;
}


/**
 * Simple class for managing the whole TrackBall-rotation-setup
 * @param canvas
 * @param State parameters
 * @returns TrackballViewer
 */
dojo.declare("gui.TrackBallViewer", null, {

    constructor: function() {

        this.t_start = (new Date()).getTime();
	
        // Flags if we have zoomed/translated:
        this.m_hasTranslated = false;
	
    
        this.orientation = quat4.create();
        this.orientation[3] = 1;
        this.projection = mat4.identity(mat4.create());
        this.modelview = mat4.identity(mat4.create());
        // Default value, will, if sane state, be set to something sane.
        this.setBoundingBox(new gui.BoundingBox("0 0 0 1 1 1"));
        this.aspect = 1;
        this.state = 1;

    },

    setSize: function(w, h) {
        this.width = w;
        this.height = h;
        this.aspect = (w + 0.0) / h;
			
        this.__generateProjectionAndTranslations();
    },

		

    updateMatrices: function()  {
        var bbmax = this.m_boundingBox.getMax();
        var bbmin = this.m_boundingBox.getMin();

        // --- set up modelview matrix
        this.modelview = mat4.identity(mat4.create());
        //this.modelview = mat4.translate(this.modelview, [0.5*(bbmin[0]+bbmax[0]),
        //    0.5*(bbmin[1]+bbmax[1]),
        //    0.5*(bbmin[2]+bbmax[2])
        //    ] );
        this.modelview = mat4.translate(this.modelview, [0,0,-this.m_translateZ]);
        this.modelview = mat4.multiply(this.modelview, quat4.toMat4(this.orientation) );
        this.modelview = mat4.translate(this.modelview,
            [-0.5*(bbmin[0]+bbmax[0]),
            -0.5*(bbmin[1]+bbmax[1]),
            -0.5*(bbmin[2]+bbmax[2])
            ]);

        // --- set up projection matrix

        // the eight corners of the bounding box
        var corners = [[ bbmin[0], bbmin[1], bbmin[2], 1.0 ],
        [ bbmin[0], bbmin[1], bbmax[2], 1.0 ],
        [ bbmin[0], bbmax[1], bbmin[2], 1.0 ],
        [ bbmin[0], bbmax[1], bbmax[2], 1.0 ],
        [ bbmax[0], bbmin[1], bbmin[2], 1.0 ],
        [ bbmax[0], bbmin[1], bbmax[2], 1.0 ],
        [ bbmax[0], bbmax[1], bbmin[2], 1.0 ],
        [ bbmax[0], bbmax[1], bbmax[2], 1.0 ]];
        // apply the modelview matrix to the eight corners to get the minimum
        // and maximum z in the camera's local coordinate system.
        var near, far;
        for( i in corners ) {
            var c = corners[i];
            var p = mat4.multiplyVec4( this.modelview, c );
            //           window.console.log( p );
            var z = (1.0/p[3])*p[2];
            if( near == null ) {
                near = z;
                far = z;
            }
            else {
                near = Math.max( near, z );
                far = Math.min( far, z );
            }
        }

        // don't let far get closer than -epsilon.
        var epsilon = 0.001;
        far = Math.min( -epsilon, far-epsilon );
        // don't let near get closer than 0.01 of far, and make sure near is
        // closer than far
        near = Math.min( 0.01*far, Math.max(far,near+epsilon) );


        // use field of view and aspect ratio to determine width and height
        var fov = Math.PI/2.0;

        var w2, h2;
        if( this.aspect > 1.0 ) {
            w2 = Math.tan( 0.5*fov )*-near;
            h2 = w2/this.aspect;
        }
        else {
            h2 = Math.tan( 0.5*fov )*-near;
            w2 = h2*this.aspect;
        }
        this.projection = mat4.frustum( -w2, w2, -h2, h2, -near, -far );
    },

    /**
     * For any boundingBox it generates a Box b such that any rotation of boundingBox is contained in b.
     * @param boundingBox
     * @returns BoundingBox
     */
    getRotationBoundingBox: function(boundingBox) {

        var max = boundingBox.getMax();
        var min = boundingBox.getMin();

        var center = [(max[0]+min[0])/2., (max[1]+min[1])/2., (max[2]+min[2])/2.];

        // Distance function (L^2-norm of difference)
        var d = function(a,b) {
            var retval = 0.0;
            for(var i = 0; i<a.length; i++) {
                retval += (a[i]-b[i])*(a[i]-b[i]);
            }
            return Math.sqrt(retval);
        }
        var radius = d(max, center);

        var newMax = [center[0]+radius, center[1]+radius, center[2]+radius];

        var newMin = [center[0]-radius, center[1]-radius, center[2]-radius];


        return new gui.BoundingBox(newMax[0]+ " " + newMax[1]+ " " + newMax[2] + " "+newMin[0]+ " " + newMin[1]+ " " + newMin[2]);



    },

    __generateProjectionAndTranslations: function( ) {
        // Set up boundingbox
        var rotBoundingBox = this.getRotationBoundingBox(this.m_boundingBox);
			
        var rotBoundingBoxMax = rotBoundingBox.getMax();
        var rotBoundingBoxMin = rotBoundingBox.getMin();
        var left = rotBoundingBoxMin[0];
        var right = rotBoundingBoxMax[0];
        var bottom = rotBoundingBoxMin[1];
        var top = rotBoundingBoxMax[1];
        var fov = Math.PI/2;
        var near = Math.max(top-bottom, right-left)/(2*Math.tan(fov/2.));

        var far = rotBoundingBoxMax[2]-rotBoundingBoxMin[2]+5.;
        if(!this.m_hasTranslated) {
            this.m_translateZ = (rotBoundingBoxMax[2]-rotBoundingBoxMin[2])/2.+near;
        }

        if (this.aspect > 1.0) {
        //this.projection = mat4.frustum(left, right, bottom / this.aspect, top / this.aspect, near, far);
        }
        else {

        //this.projection = mat4.frustum(left, right, bottom, top, near, far);
        }

        this.m_translateBoundingBoxCenter =  [-(rotBoundingBoxMin[0]+rotBoundingBoxMax[0])/2, 
        -(rotBoundingBoxMin[1]+rotBoundingBoxMax[1])/2, 
        -(rotBoundingBoxMax[2]+rotBoundingBoxMin[2])/2];
			
        // We will need this variable for  zooming.
        this.near = near;
    },


    setBoundingBox: function(boundingBox) {
        if(typeof boundingBox== "string") {
            boundingBox = gui.BoundingBox(boundingBox);
        }
        this.m_boundingBox = boundingBox;
        this.__generateProjectionAndTranslations();
    },


    pointOnUnitSphere: function(x, y) {
        var nx = ((2.0 * x) / this.width - 1.0) * this.aspect;
        var ny = -((2.0 * y) / this.height - 1.0);
        var r2 = nx * nx + ny * ny;

        if (r2 < 1.0) {
            return vec3.create([nx, ny, Math.sqrt(1.0 - r2)]);
        }
        else {
            var r = 1.0 / Math.sqrt(r2);
            return vec3.create([r * nx, r * ny, 0.0]);
        }
    },

    rotationBegin: function(x, y) {
        this.begin_orientation = quat4.create(this.orientation);
        this.begin_dir = this.pointOnUnitSphere(x, y);
        this.state = 1;
    },

		
    rotationEnd: function(x, y) {
        this.end= quat4.create(this.orientation);
        this.end_dir = this.pointOnUnitSphere(x, y);
        this.state = 0;
    },
    
    
    zoomEnd: function(x, y) {
        this.state = 0;
    },
		
 
    mouseMove: function(x, y) {
        if (this.state == 1) {
            var axis = vec3.create(this.begin_dir);
            var curr_dir = this.pointOnUnitSphere(x, y);
            axis = vec3.cross(axis, curr_dir);

            var l = vec3.length(axis);
            if (Math.abs(l) > 1e-8) {
                vec3.normalize(axis);
                var a = Math.acos(vec3.dot(curr_dir, this.begin_dir));
                var q = axisAngle(axis, a);
                this.orientation = quat4.create(this.begin_orientation);
                this.orientation = quat4.multiply(this.orientation, q);
            }
        }
        else if(this.state == 2) {
            this.mouseZoom(x, y);
        }
    },

   

    __str_mat: function(matr) {
        // Keep it simple:
        return mat4.str(matr).replace(/,/gi,"").replace("[", "").replace("]","");
    },

 
    zoomBegin: function(x, y) {
        this.state = 2;
        this.m_zoom_start = [x,y];
        this.m_translateZ_begin = this.m_translateZ
    },
		
    mouseZoom: function(x, y) {
        this.m_hasTranslated = true;
			
        var magicScalingFactor = this.near/this.height;
			
        this.m_translateZ = this.m_translateZ_begin-(this.m_zoom_start[1] -y)*magicScalingFactor;
    },
    
    zoomFactorBegin: function(factor) {
        this.state = 2;
        this.m_translateZ_begin = this.m_translateZ
    },
    zoomFactor: function(factor) {
        this.m_hasTranslated = true;
        var bbmax = this.m_boundingBox.getMax();
        var bbmin = this.m_boundingBox.getMin();

        factor = 1 - factor;
        this.m_translateZ = this.m_translateZ_begin +  factor/10*(bbmax[2]-bbmin[2]);


    },

    getViewCoordSys: function() {
        var view_coord_sys = {
            m_projection	: this.projection,
            m_projection_inverse : mat4.inverse(mat4.create(this.projection)),
            m_to_world		: mat4.inverse(mat4.create(this.modelview)),
            m_from_world	: this.modelview
        }
        return view_coord_sys;
    },
    
    getProjectionStr : function() {
        return this.__str_mat(this.projection);
    },
    
    getModelViewStr: function() {
        return this.__str_mat(this.modelview);
    },
    
    getProjection : function() {
        return this.projection;
    },
    
    getModelView: function() {
        return this.modelview;
    }

});

/**
 * Creates a bounding box specified by the string
 * @param stringBoundingBox string expects 6 floats representing two vectors in R^3. 
 * 		The former being the max corner, the latter the min corner.
 */
dojo.declare("gui.BoundingBox", null, {
    constructor: function(stringBoundingBox) {
        var tmpArray = stringBoundingBox.split(" ");
	
        this.m_max = [tmpArray[0]-0.0, 
        tmpArray[1]-0.0, tmpArray[2]-0.0];
        this.m_min = [tmpArray[3]-0.0,
        tmpArray[4]-0.0,tmpArray[5]-0.0];
        if(this.m_min[0] > this.m_max[0] || this.m_min[1] > this.m_max[1] || this.m_min[2] > this.m_max[2]) {
            var tmp = this.m_min;
            this.m_min = this.m_max;
            this.m_max = tmp;
        }
    },
    getMax: function() {
        return this.m_max;
    },
    getMin: function() {
        return this.m_min;
    }
});
