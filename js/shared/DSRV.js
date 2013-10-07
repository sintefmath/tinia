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
    q[3] = c;

    return q;
}
function DSRV(parameters) {
    console.log("constructing dsrv");
    this.m_exposedModel = parameters.exposedModel;
    this.m_key = parameters.key;
    this.m_boundingBoxKey = parameters.boundingBoxKey;

    this.m_orientation = quat4.identity();


    this.m_projection = mat4.identity(mat4.create());
    this.m_modelview = mat4.identity(mat4.create());

    this.getBoundingBoxFromModel();

    this.m_aspect = 1;

    this.m_state = -1;
    this.ROTATE = 0;
    this.ZOOM = 1;

    this.m_beginDirection = vec3.create([0,0,0]);

    this.m_translateZ = 4;

    // Get width and height:
    this.setSize(this.m_exposedModel.getElementValue(this.m_key).getElementValue("width"),
            this.m_exposedModel.getElementValue(this.m_key).getElementValue("height"));

    // Handle updates to the boundingbox:

    this.m_exposedModel.addLocalListener(this.m_boundingBoxKey, tinia.hitch(this, function(key, bb) {
        this.updateBoundingBox(bb);
        this.updateMatrices();
        this.insertMatrices();
    }));

    this.m_exposedModel.addLocalListener(this.m_key, tinia.hitch(this, function(key, viewer) {
        var height = viewer.getElementValue("height");
        var width = viewer.getElementValue("width");
        if(height !== this.m_height || width !== this.m_width) {
            this.setSize(width, height);
            this.updateMatrices();
            this.insertMatrices();
        }
    }));

    
    this.updateMatrices();
    this.insertMatrices();

    console.log("DSRV Constructed");

}


DSRV.prototype = {
    updateBoundingBox: function (bb) {
        bb = bb.split(" ");

        this.m_bbmin = vec3.create([bb[0] - 0.0, bb[1] - 0.0, bb[2] - 0.0]);
        this.m_bbmax = vec3.create([bb[3] - 0.0, bb[4] - 0.0, bb[5] - 0.0]);

        // This is used to scale zooming levels
        this.m_maxLength = Math.max(this.m_bbmax[0] - this.m_bbmin[0],
            this.m_bbmax[1] - this.m_bbmin[1],
            this.m_bbmax[2] - this.m_bbmin[1]);
    },

    getBoundingBoxFromModel: function () {
        this.updateBoundingBox(this.m_exposedModel.getElementValue(this.m_boundingBoxKey));
    },

    setSize: function (w, h) {

        this.m_width = w - 0.0;
        this.m_height = h - 0.0;
        this.m_aspect = (w - 0.0) / h;
    },
    mouseMoveEvent: function (event) {
        switch (this.m_state) {
            case this.ROTATE:
                this.handleRotateMove(event.relativeX, event.relativeY);

                break;
            case this.ZOOM:
                this.handleZoomMove(event.relativeX, event.relativeY);
                break;
        }

        if (this.m_state > -1) {
            this.updateMatrices();
            this.insertMatrices();
        }
    },

    mousePressEvent: function (event) {
        console.log("PRESS: " + event.relativeX + ", " + event.relativeY);

        // CTRL + Left mouse button is zoom.
        if (event.ctrlKey && event.button === 0) {
            console.log("Zooming start");
            this.m_zoomStart = event.relativeY;
            this.m_translateZBegin = this.m_translateZ;
            this.m_state = this.ZOOM;
            return;
        }
        switch (event.button) {
            case this.ROTATE:
                this.m_beginOrientation = quat4.create(this.m_orientation);
                this.m_beginDirection = this.pointOnUnitSphere(event.relativeX, event.relativeY);
                this.m_state = this.ROTATE;
                break;
            case this.ZOOM:
                console.log("Zooming start");
                this.m_zoomStart = event.relativeY;
                this.m_translateZBegin = this.m_translateZ;
                this.m_state = this.ZOOM;
                break;
            default:
                this.m_state = -1;
        }
    },

    mouseReleaseEvent: function (event) {
        console.log("released");
        this.m_state = -1;
    },

    keyPressEvent: function (event) {
        console.log(event.key);
    },

    touchStartEvent: function (event) {

        this.mouseDownEvent(event);
    },

    touchEndEvent: function (event) {
        this.mouseUpEvent(event);
    },

    touchMoveEvent: function (event) {
        this.mouseMoveEvent(event);
        console.log("Moving");
    },

    handleRotateMove: function (x, y) {
        var axis = vec3.create(this.m_beginDirection);
        var curr_dir = this.pointOnUnitSphere(x, y);
        axis = vec3.cross(axis, curr_dir);

        var l = vec3.length(axis);
        if (Math.abs(l) > 1e-8) {
            axis = vec3.normalize(axis);
            var a = Math.acos(vec3.dot(this.m_beginDirection, curr_dir));
            var q = quat4.fromAngleAxis(a, axis);
            this.m_orientation = quat4.create(this.m_beginOrientation);
            this.m_orientation = quat4.multiply(q, this.m_orientation);
        }
    },

    handleZoomMove: function (x, y) {
        console.log("Zooming move");
        var scale = this.m_maxLength || this.m_height ? this.m_maxLength / this.m_height : 1;
        this.m_translateZ = this.m_translateZBegin - (y - this.m_zoomStart) * scale;
        console.log(this.m_translateZ);
    },

    pointOnUnitSphere: function (x, y) {
        var nx = (x / this.m_width - .5) * this.m_aspect;
        var ny = -(y / this.m_height - .5);
        var r2 = nx * nx + ny * ny;

        if (r2 < 1) {
            return vec3.create([nx, ny, Math.sqrt(1.0 - r2)]);
        }
        else {
            var r = 1.0 / Math.sqrt(r2);
            return vec3.create([r * nx, r * ny, 0.0]);
            //return vec3.create(nx, ny, 0.5/Math.sqrt(r2));
        }
    },

    updateMatrices: function () {
        var bbmax = this.m_bbmax;
        var bbmin = this.m_bbmin;

        // --- set up modelview matrix
        this.m_modelview = mat4.identity(mat4.create());
        this.m_modelview = mat4.translate(this.m_modelview, [0, 0, -this.m_translateZ]);
        this.m_modelview = mat4.multiply(this.m_modelview, quat4.toMat4(this.m_orientation));
        this.m_modelview = mat4.translate(this.m_modelview,
                                         [-0.5 * (bbmin[0] + bbmax[0]),
                                          -0.5 * (bbmin[1] + bbmax[1]),
                                          -0.5 * (bbmin[2] + bbmax[2])
                                         ]);

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
            //           window.console.log( p );
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

        // don't let far get closer than -epsilon.
        var epsilon = 0.001;
        far = Math.min(-epsilon, far - epsilon);
        // don't let near get closer than 0.01 of far, and make sure near is
        // closer than far
        near = Math.min(0.01 * far, Math.max(far, near + epsilon));


        // use field of view and aspect ratio to determine width and height
        var fov = Math.PI / 2.0;

        var w2, h2;
        if (this.m_aspect > 1.0) {
            w2 = Math.tan(0.5 * fov) * -near;
            h2 = w2 / this.m_aspect;
        }
        else {
            h2 = Math.tan(0.5 * fov) * -near;
            w2 = h2 * this.m_aspect;
        }

        //this.m_projection = mat4.frustum(-w2, w2, -h2, h2, -near, -far);
        this.m_projection = mat4.perspective(90, this.m_aspect, -near, -far);
    },

    insertMatrices: function () {

        var viewer = this.m_exposedModel.getElementValue(this.m_key);
        viewer.updateElement("modelview", this.m_modelview);
        viewer.updateElement("projection", this.m_projection);
        this.m_exposedModel.updateElement(this.m_key, viewer);
    }
}
