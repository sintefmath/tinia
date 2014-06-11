/* Copyright STIFTELSEN SINTEF 2014
 *
 * This file is part of the Tinia Framework.
 * FRView is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the FRView.  If not, see <http://www.gnu.org/licenses/>.
 */

function TrackBallManipulator()
{
    this.STATE_VOID = 0;
    this.STATE_ROTATION = 1;
    this.STATE_TRANSLATION = 2;
    this.STATE_DOLLY = 3;
    this.m_state = this.STATE_VOID;
    this.m_width = 1;
    this.m_height = 1;

    this.m_fov = Math.PI/2.0;
    this.m_aspect = 1.0;
    this.m_factor = 1.0;
    this.m_coi_ndc_z = 0.5;
    this.m_modelview_projection_inverse_prev = mat4.create();
    this.m_axis = vec3.create();
    this.m_pos = vec3.create();
    this.m_quat = quat4.create();
    this.m_pos_prev = vec3.create();
    this.m_distance = 1.0;
    this.m_distance_prev = 1.0;
    this.m_bbox_min = vec3.createFrom( -1.0, -1.0, -1.0 );
    this.m_bbox_max = vec3.createFrom(  1.0,  1.0,  1.0 );
    this.m_coi = vec3.createFrom( 0.0,  0.0, 0.0 );
    this.m_coi_prev = vec3.create();
    this.m_orientation = quat4.createFrom( 0.0, 0.0, 0.0, 1.0 );
    this.m_orientation_prev = quat4.create();
    this.m_projection = mat4.create();
    this.m_modelview = mat4.create();

    this.viewAll();
}
TrackBallManipulator.prototype = {

    setWindowSize: function( width, height )
    {
        if( (width !== 0) && (height !== 0) ) {
            if( (this.m_width !== width ) || (this.m_height !== height) ) {
                this.m_width = width;
                this.m_height = height;
                this.m_aspect = (width+0.0)/height;
                this.updateMatrices();
                return true;
            }
        }
        return false;
    },
    setBoundingBox: function( min, max )
    {
        vec3.set( min, this.m_bbox_min );
        vec3.set( max, this.m_bbox_max );
        this.m_factor = Math.max( (this.m_bbox_max[0]-this.m_bbox_min[0]),
                                  (this.m_bbox_max[1]-this.m_bbox_min[1]),
                                  (this.m_bbox_max[2]-this.m_bbox_min[2]) );
    },
    viewAll: function()
    {
        // set center-of-interest in center of bounding box
        this.m_coi = vec3.set( [0.5*(this.m_bbox_min[0]+this.m_bbox_max[0]),
                   0.5*(this.m_bbox_min[1]+this.m_bbox_max[1]),
                   0.5*(this.m_bbox_min[2]+this.m_bbox_max[2])],
                 this.m_coi );

        // push camera distance such that bounding box is just inside the view
        var t = 1.0/Math.tan( 0.5*this.m_fov );
        var p = vec3.create();
        this.m_distance = 0.0;

        for(var i=0; i<8; i++ ) {
            p[0] = (( i   %2)==0) ? this.m_bbox_min[0] : this.m_bbox_max[0];
            p[1] = ((Math.floor(i/2)%2)==0) ? this.m_bbox_min[1] : this.m_bbox_max[1];
            p[2] = ((Math.floor(i/4)%2)==0) ? this.m_bbox_min[2] : this.m_bbox_max[2];
            vec3.subtract( p, this.m_coi );
            quat4.multiplyVec3( this.m_orientation, p );
            this.m_distance = Math.max( this.m_distance,
                                        p[2]+t*Math.max(Math.abs( p[0] ), Math.abs(p[1])) );
        }
        this.updateMatrices();
    },
    updateMatrices: function()
    {
        // create modelview
        this.m_modelview = mat4.fromRotationTranslation( this.m_orientation,
                                                         [0.0, 0.0, -this.m_distance ],
                                                         this.m_modelview );
        this.m_modelview = mat4.translate( this.m_modelview,
                                          [ -this.m_coi[0], -this.m_coi[1], -this.m_coi[2] ] );

        // project all corners of bounding box to find near and far
        var p = vec4.create();
        var near, far;
        for(var i=0; i<8; i++ ) {
            p[0] = (( i   %2)==0) ? this.m_bbox_min[0] : this.m_bbox_max[0];
            p[1] = ((Math.floor(i/2)%2)==0) ? this.m_bbox_min[1] : this.m_bbox_max[1];
            p[2] = ((Math.floor(i/4)%2)==0) ? this.m_bbox_min[2] : this.m_bbox_max[2];
            p[3] = 1.0;
            p = mat4.multiplyVec4( this.m_modelview, p );
            var z = (1.0/p[3])*p[2];
            if( i==0) {
                near = z;
                far = z;
            }
            else {
                near = Math.max( near, z );
                far  = Math.min( far,  z );
            }
        }
        // sanity checks for near and far
        var epsilon = 0.001;
        far  = Math.min( -epsilon, far-epsilon );
        near = Math.min( 0.01*far, Math.max( far, near+epsilon) );
        // create projection
        var w2, h2;
        if( this.m_aspect < 1.0 ) {
            w2 = Math.tan( 0.5*this.m_fov )*-near;
            h2 = w2/this.m_aspect;
        }
        else {
            h2 = Math.tan( 0.5*this.m_fov )*-near;
            w2 = h2*this.m_aspect;
        }
        this.m_projection = mat4.frustum( -w2, w2,
                                          -h2, h2,
                                          -near, -far );
    },
    modelviewMatrix: function()
    {
        return this.m_modelview;
    },
    projectionMatrix: function()
    {
        return this.m_projection;
    },
    setStateRotation: function( x, y )
    {
        this.m_state = this.STATE_ROTATION;
        this._pointOnUnitSphere( x, y, this.m_pos_prev );
        quat4.set( this.m_orientation, this.m_orientation_prev );
    },
    setStateTranslation: function( x, y )
    {
        this.m_state = this.STATE_TRANSLATION;
        vec3.set( this.m_coi, this.m_coi_prev );

        mat4.multiply( this.m_projection,
                       this.m_modelview,
                       this.m_modelview_projection_inverse_prev );
        // project center of interest to normalized device coords to get which
        // depth we will translate at.
        var p = vec4.createFrom( this.m_coi[0],
                                 this.m_coi[1],
                                 this.m_coi[2],
                                 1.0 );
        mat4.multiplyVec4( this.m_modelview_projection_inverse_prev, p );
        this.m_coi_ndc_z = p[2]/p[3];

        // Transform current mouse pos at COI depth to object space
        this._normalizedPos( x, y, this.m_pos_prev );
        mat4.inverse( this.m_modelview_projection_inverse_prev );
        vec4.set( [this.m_pos_prev[0],
                   this.m_pos_prev[1],
                   this.m_coi_ndc_z,
                   1.0 ],
                   p );
        mat4.multiplyVec4( this.m_modelview_projection_inverse_prev, p );
        vec3.set( [p[0]/p[3],
                   p[1]/p[3],
                   p[2]/p[3] ],
                  this.m_pos_prev );
    },
    setStateDolly: function( x, y )
    {
        this.m_state = this.STATE_DOLLY;
        this._normalizedPos( x, y, this.m_pos_prev );
        this.m_distance_prev = this.m_distance;
    },
    setStateNone: function( x, y )
    {
        this.m_state = this.STATE_VOID;
    },
    mouseDrag: function( x, y )
    {
        if( this.m_state == this.STATE_ROTATION ) {
            this._pointOnUnitSphere( x, y, this.m_pos );
            if( Math.abs( vec3.dot( this.m_pos_prev, this.m_pos ) ) < 0.999999 ) {
                vec3.rotationTo( this.m_pos_prev,
                                 this.m_pos,
                                 this.m_quat );
                quat4.multiply( this.m_quat,
                                this.m_orientation_prev,
                                this.m_orientation );
                this.updateMatrices();
                return true;
            }
        }
        else if( this.m_state == this.STATE_TRANSLATION ) {
            this._normalizedPos( x, y, this.m_pos );
            var p = vec4.create();
            vec4.set( [this.m_pos[0],
                       this.m_pos[1],
                       this.m_coi_ndc_z,
                       1.0 ],
                       p );
            mat4.multiplyVec4( this.m_modelview_projection_inverse_prev, p );
            vec3.set( [ this.m_coi_prev[0] - p[0]/p[3] + this.m_pos_prev[0],
                        this.m_coi_prev[1] - p[1]/p[3] + this.m_pos_prev[1],
                        this.m_coi_prev[2] - p[2]/p[3] + this.m_pos_prev[2] ],
                       this.m_coi );
            this.updateMatrices();
            return true;
        }
        else if( this.m_state == this.STATE_DOLLY ) {
            this._normalizedPos( x, y, this.m_pos );
            this.m_distance = Math.max( 0.001*this.m_factor,
                                        this.m_distance_prev + this.m_factor*(this.m_pos[1]-this.m_pos_prev[1]) );
            this.updateMatrices();
            return true;
        }
        return false;
    },
    _normalizedPos: function( x, y, vec )
    {
        vec[0] =  ((2.0 * x) / this.m_width - 1.0) * this.m_aspect;
        vec[1] = -((2.0 * y) / this.m_height - 1.0);
        vec[2] = 1.0;
        return vec;
    },
    _pointOnUnitSphere: function( x, y, vec )
    {
        var nx = ((2.0 * x) / this.m_width - 1.0) * this.m_aspect;
        var ny = -((2.0 * y) / this.m_height - 1.0);
        var r2 = nx * nx + ny * ny;
        if (r2 < 1.0) {
            vec3.set( [nx, ny, Math.sqrt(1.0 - r2)], vec );
        }
        else {
            var r = 1.0 / Math.sqrt(r2);
            vec3.set([r * nx, r * ny, 0.0], vec );
        }
        return vec;
    }

}

function DSRV( params ) {
    this.m_model        = params.exposedModel;
    this.m_key          = params.key;
    this.m_bbox_key     = params.boundingBoxKey;
    this.m_manipulator  = new TrackBallManipulator();

    this.m_model.addLocalListener( this.m_key,
                                   tinia.hitch( this,
                                               function( key, viewer )
    {
        var w = viewer.getElementValue("width");
        var h = viewer.getElementValue("height");
        if( this.m_manipulator.setWindowSize( w, h ) ) {
            this.pushMatrices();
        }
    } ) );
    if( this.m_bbox_key ) {
        this.m_model.addLocalListener( this.m_bbox_key,
                                       tinia.hitch( this,
                                                    function( key, viewer )
        {
            var bbox_str = viewer.getElementValue( key );
            this.setBoundingBox( bbox_str );
        } ) );
        this.setBoundingBox( this.m_model.getElementValue( this.m_bbox_key ) );
    }
}

DSRV.prototype = {
    mousePressEvent: function (event) {
        var x = event.relativeX;
        var y = event.relativeY;
        switch( event.button ) {
        case 0:
            this.m_manipulator.setStateRotation( x, y );
            break;
        case 1:
            this.m_manipulator.setStateTranslation( x, y );
            break;
        case 2:
            this.m_manipulator.setStateDolly( x, y );
            break;
        }
        this.pushMatrices();
    },
    setBoundingBox: function( boundingbox_string ) {
        var t = boundingbox_string.split(" ");
        if( t.length >= 6 ) {
            var min = [ Math.min( t[0]-0.0, t[3]-0.0),
                        Math.min( t[1]-0.0, t[4]-0.0),
                        Math.min( t[2]-0.0, t[5]-0.0) ];
            var max = [ Math.max( t[0]-0.0, t[3]-0.0),
                        Math.max( t[1]-0.0, t[4]-0.0),
                        Math.max( t[2]-0.0, t[5]-0.0) ];
            this.m_manipulator.setBoundingBox( min, max );
            this.m_manipulator.viewAll();
            this.pushMatrices();
        }
    },
    mouseMoveEvent: function(event) {
        var x = event.relativeX;
        var y = event.relativeY;
        if( this.m_manipulator.mouseDrag( x, y ) ) {
            this.pushMatrices();
        }
    },
    mouseReleaseEvent: function(event) {
        var x = event.relativeX;
        var y = event.relativeY;
        this.m_manipulator.setStateNone( x, y );
        //this.pushMatrices();
    },

    touchStartEvent: function (event)
    {
        if( event.touches.length == 1 ) {
            event.button = this.ROTATE;
            this.mousePressEvent(event);
        }
        else if( event.touches.length == 2 ) {
            event.button = this.ZOOM;
            this.mousePressEvent(event);
        }
    },
    touchEndEvent: function (event)
    {
        if( event.touches.length == 1 ) {
            event.button = this.ROTATE;
            this.mouseReleaseEvent(event);
        }
        else if( event.touches.length == 2 ) {
            event.button = this.ZOOM;
            this.mouseReleaseEvent(event);
        }
    },
    touchMoveEvent: function (event)
    {
        if( event.touches.length == 1 ) {
            event.button = this.ROTATE;
            this.mouseMoveEvent(event);
        }
        else if(event.touches.length == 2) {
            event.button = this.ZOOM;
            this.mouseMoveEvent(event);
        }
    },
    pushMatrices: function() {
        var viewer = this.m_model.getElementValue( this.m_key );
        viewer.updateElement( "modelview", this.m_manipulator.modelviewMatrix() );
        viewer.updateElement( "projection", this.m_manipulator.projectionMatrix() );
        this.m_model.updateElement( this.m_key, viewer );   // needed?
    }
}
