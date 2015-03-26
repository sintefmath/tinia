/* Copyright STIFTELSEN SINTEF 2014
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

dojo.provide("gui.ProxyModel");

dojo.declare("gui.ProxyModel", null, {


    constructor: function(glContext) {
        this._gl                = glContext;
        this.depthTexture       = this._gl.createTexture();
        this.rgbTexture         = this._gl.createTexture();
        this.projection         = null;
        this.projection_inverse = null;
        this.from_world         = null;     // The "view" matrix
        this.to_world           = null;     // And its inverse
        this.dir                = null;     // Direction in which the camera points
        this.dist               = null;     // Distance from camera to the origin
        this.state              = 0;        // 0) loading of data not started, object is not in use, 1) loading going on, 2) loading done, ready for use
    },


    setAll: function(depthBufferAsText, imageAsText, viewMatAsText, projMatAsText) {
        if ( this.state == 1 ) {
            throw "Trying to set data for a proxy model being processed!";
        }

        if ( (!depthBufferAsText) || (!viewMatAsText) || (!projMatAsText) ) {
            // then we have probably been passed a snapshot made when autoProxy was disabled. We cannot and should not use this, so we just return.
            return;
        }

        this.state = 1;
        var imagesLoaded = 0;
        var depth_t0 = 0;
        var rgb_t0 = 0;

        var depthImage = new Image();
        depthImage.onload = dojo.hitch(this, function() {
            this._gl.bindTexture(this._gl.TEXTURE_2D, this.depthTexture);
            this._gl.texImage2D(this._gl.TEXTURE_2D, 0, this._gl.RGB, this._gl.RGB, this._gl.UNSIGNED_BYTE, depthImage);
            this._gl.texParameteri(this._gl.TEXTURE_2D, this._gl.TEXTURE_MAG_FILTER, this._gl.NEAREST);
            this._gl.texParameteri(this._gl.TEXTURE_2D, this._gl.TEXTURE_MIN_FILTER, this._gl.NEAREST);
            this._gl.bindTexture(this._gl.TEXTURE_2D, null);
            // console.log("ProxyModel.setAll: depth size = " + depthImage.width + " x " + depthImage.height);
            // console.log("Depth image (" + depthBufferAsText.length + " bytes) loaded in " + (Date.now()-depth_t0) + " ms. (" + Math.floor((depthBufferAsText.length/(Date.now()-depth_t0))) + " bytes/ms)");
            imagesLoaded = imagesLoaded + 1;
            if (imagesLoaded==2) {
                this.projection         = projMatAsText.split(/ /);
                this.projection_inverse = mat4.inverse(mat4.create( this.projection ));
                this.from_world         = viewMatAsText.split(/ /);
                this.to_world           = mat4.inverse(mat4.create( this.from_world ));
                this.dir = vec3.create( [-this.to_world[8], -this.to_world[9], -this.to_world[10]] );
                vec3.normalize(this.dir); // Should probably already be normalized...
                this.dist = vec3.length( vec3.create( [this.to_world[12], this.to_world[13], this.to_world[14]] ) );
                this.state = 2;
            }
        });
        depth_t0 = Date.now();
        // console.log("Starting depth image loading");
        depthImage.src = "data:image/png;base64," + depthBufferAsText;

        var rgbImage = new Image();
        rgbImage.onload = dojo.hitch(this, function() {
            // console.log("ProxyModel.setAll: RGB size = " + rgbImage.width + " x " + rgbImage.height);
            var pot_width  = parseInt(Math.pow(2.0, Math.floor(Math.log(rgbImage.width-1)/Math.log(2.0))+1));     // This *may* have numerical problems... not quite sure how to (elegantly) avoid this...
            var pot_height = parseInt(Math.pow(2.0, Math.floor(Math.log(rgbImage.height-1)/Math.log(2.0))+1));
            // console.log("                   proposed new RGB size = " + pot_width + " x " + pot_height);
            if ( (pot_width!=rgbImage.width) || (pot_height!=rgbImage.height) ) {
                // Only in these cases do we bother with (possibly costly and/or quality degrading) rescaling
                var canvas = document.createElement('canvas');
                var ctx = canvas.getContext('2d');
                canvas.width=pot_width;
                canvas.height=pot_height;
                ctx.drawImage(rgbImage, 0, 0, pot_width, pot_height);
                rgbImage = canvas;
            }
            this._gl.bindTexture(this._gl.TEXTURE_2D, this.rgbTexture);
            this._gl.texImage2D(this._gl.TEXTURE_2D, 0, this._gl.RGB, this._gl.RGB, this._gl.UNSIGNED_BYTE, rgbImage);
            this._gl.texParameteri(this._gl.TEXTURE_2D, this._gl.TEXTURE_MAG_FILTER, this._gl.LINEAR);
            this._gl.texParameteri(this._gl.TEXTURE_2D, this._gl.TEXTURE_MIN_FILTER, this._gl.LINEAR);
            this._gl.bindTexture(this._gl.TEXTURE_2D, null);
//            console.log("RGB image (" + imageAsText.length + " bytes) loaded in " + (Date.now()-rgb_t0) + " ms. (" + Math.floor((imageAsText.length/(Date.now()-rgb_t0))) + " bytes/ms)");
            imagesLoaded = imagesLoaded + 1;
            if (imagesLoaded==2) {
                this.projection         = projMatAsText.split(/ /);
                this.projection_inverse = mat4.inverse(mat4.create( this.projection ));
                this.from_world         = viewMatAsText.split(/ /);
                this.to_world           = mat4.inverse(mat4.create( this.from_world ));
                this.dir = vec3.create( [-this.to_world[8], -this.to_world[9], -this.to_world[10]] );
                vec3.normalize(this.dir); // Should probably already be normalized...
                this.dist = vec3.length( vec3.create( [this.to_world[12], this.to_world[13], this.to_world[14]] ) );
                this.state = 2;
            }
        });
        rgb_t0 = Date.now();
        rgbImage.src = "data:image/png;base64," + imageAsText;

    },


});
