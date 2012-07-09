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
function DSRV(parameters) {
    this.m_exposedModel = parameters.exposedModel;
    this.m_key = parameters.key;
    this.m_boundingBoxKey = parameters.boundingBoxKey;

    this.m_orientation = quat4.create();
    this.m_orientation[3] = 1;

    this.m_projection = mat4.identity(mat4.create());
    this.m_modelview = mat4.identity(mat4.create());

    this.getBoundingBoxFromModel();

    this.m_aspect = 1;

    this.m_state = -1;
    this.ROTATE = 0;
    this.ZOOM = 1;

    this.m_beginDirection = vec3.create();

    this.m_translateZ = 4;

    // Get width and height:
    this.setSize(this.m_exposedModel.getElementValue(this.m_key).getElementValue("width"),
            this.m_exposedModel.getElementValue(this.m_key).getElementValue("height"));

    // Handle updates to the boundingbox:
    var DSRV_this = this;
    this.m_exposedModel.addListener(this.m_boundingBoxKey, function(bb) {
        DSRV_this.updateBoundingBox(bb);
    });

    this.updateMatrices();
    this.insertMatrices();
}

DSRV.prototype = {
    updateBoundingBox : function(bb) {
        bb = bb.split(" ");
        this.m_bbmin = vec3.createFrom(bb[0] - 0.0, bb[1] - 0.0, bb[2] - 0.0);
        this.m_bbmax = vec3.createFrom(bb[3] - 0.0, bb[4] - 0.0, bb[5] - 0.0);
    },

    getBoundingBoxFromModel : function() {
        this.updateBoundingBox(this.m_exposedModel.getElementValue(this.m_boundingBoxKey));
    },

    setSize : function(w, h) {

        this.m_width = w;
        this.m_height = h;
        this.m_aspect = (w + 0.0) / h;
    },
    mouseMoveEvent : function(event) {
        switch(this.m_state) {
        case this.ROTATE:
            print("rotation");
            this.handleRotateMove(event.x, event.y);

            break;
        case this.ZOOM:
            this.handleZoomMove(event.x, event.y);
            break;
        }

        this.updateMatrices();
        this.insertMatrices();
    },

    mousePressEvent : function(event) {
        switch(event.button) {
        case this.ROTATE:
            print("BEGIN ROTATE");
            this.m_beginOrientation = quat4.create(this.m_orientation);
            this.m_beginDirection = this.pointOnUnitSphere(event.x, event.y);
            this.m_state = this.ROTATE;
            break;
        default:
            this.m_state = -1;
        }

        print(event.x + ", " + event.y);
    },

    mouseReleaseEvent : function(event) {
        print(event.x + ", " + event.y);
    },

    handleRotateMove: function(x, y) {
        var axis = vec3.create(this.m_beginDirection);
        var curr_dir = this.pointOnUnitSphere(x, y);
        axis = vec3.cross(axis, curr_dir);

        var l = vec3.length(axis);
        if (Math.abs(l) > 1e-8) {
            vec3.normalize(axis);
            var a = Math.acos(vec3.dot(curr_dir, this.m_beginDirection));
            var q = axisAngle(axis, a);
            this.m_orientation = quat4.create(this.m_beginOrientation);
            this.m_orientation = quat4.multiply(this.m_orientation, q);
        }
    },

    pointOnUnitSphere: function(x, y) {
        var nx = ((2.0 * x) / this.m_width - 1.0) * this.m_aspect;
        var ny = -((2.0 * y) / this.m_height - 1.0);
        var r2 = nx * nx + ny * ny;

        if (r2 < 1.0) {
            return vec3.create([nx, ny, Math.sqrt(1.0 - r2)]);
        }
        else {
            var r = 1.0 / Math.sqrt(r2);
            return vec3.create([r * nx, r * ny, 0.0]);
        }
    },

    updateMatrices: function()  {
        var bbmax = this.m_bbmax;
        var bbmin = this.m_bbmin;

        // --- set up modelview matrix
        this.m_modelview = mat4.identity(mat4.create());
        this.m_modelview = mat4.translate(this.m_modelview, [0, 0, -this.m_translateZ]);
        this.m_modelview = mat4.multiply(this.m_modelview, quat4.toMat4(this.m_orientation) );
        this.m_modelview = mat4.translate(this.m_modelview,
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
            var p = mat4.multiplyVec4( this.m_modelview, c );
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
        if( this.m_aspect > 1.0 ) {
            w2 = Math.tan( 0.5*fov )*-near;
            h2 = w2/this.m_aspect;
        }
        else {
            h2 = Math.tan( 0.5*fov )*-near;
            w2 = h2*this.m_aspect;
        }
        this.m_projection = mat4.frustum( -w2, w2, -h2, h2, -near, -far );
    },

    insertMatrices: function() {
        var viewer = this.m_exposedModel.getElementValue(this.m_key);
        viewer.updateElement("modelviewMatrix", this.m_modelview);
        viewer.updateElement("projectionMatrix", this.m_projection);
        this.m_exposedModel.updateElement(this.m_key, viewer);
    }
}
