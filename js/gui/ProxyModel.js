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
        this.state = 1;

        var image = new Image();
        image.onload = dojo.hitch(this, function() {
            this._gl.bindTexture(this._gl.TEXTURE_2D, this.depthTexture);
            this._gl.texImage2D(this._gl.TEXTURE_2D, 0, this._gl.RGB, this._gl.RGB, this._gl.UNSIGNED_BYTE, image);
            this._gl.bindTexture(this._gl.TEXTURE_2D, null);
            this._setDepthSamplingMode(this._gl.NEAREST);

            var rgbImage = new Image();
            rgbImage.onload = dojo.hitch(this, function() {
                this._gl.bindTexture(this._gl.TEXTURE_2D, this.rgbTexture);
                this._gl.texImage2D(this._gl.TEXTURE_2D, 0, this._gl.RGBA, this._gl.RGBA, this._gl.UNSIGNED_BYTE, rgbImage);
                this._gl.bindTexture(this._gl.TEXTURE_2D, null);
                this._setRgbSamplingMode(this._gl.NEAREST);

                this.projection         = projMatAsText.split(/ /);
                this.projection_inverse = mat4.inverse(mat4.create( this.projection ));
                this.from_world         = viewMatAsText.split(/ /);
                this.to_world           = mat4.inverse(mat4.create( this.from_world ));

                this.dir = vec3.create( [-this.to_world[8], -this.to_world[9], -this.to_world[10]] );
                vec3.normalize(this.dir); // Should probably already be normalized...
                this.dist = vec3.length( vec3.create( [this.to_world[12], this.to_world[13], this.to_world[14]] ) );

                this.state = 2;
            });
            rgbImage.src = "data:image/png;base64," + imageAsText;

        });
        image.src = "data:image/png;base64," + depthBufferAsText;
    },


    _setDepthSamplingMode: function(mode) {
        this._gl.bindTexture(this._gl.TEXTURE_2D, this.depthTexture);
        this._gl.texParameteri(this._gl.TEXTURE_2D, this._gl.TEXTURE_MAG_FILTER, mode);
        this._gl.texParameteri(this._gl.TEXTURE_2D, this._gl.TEXTURE_MIN_FILTER, mode);
        this._gl.bindTexture(this._gl.TEXTURE_2D, null);
    },


    _setRgbSamplingMode: function(mode) {
        this._gl.bindTexture(this._gl.TEXTURE_2D, this.rgbTexture);
        this._gl.texParameteri(this._gl.TEXTURE_2D, this._gl.TEXTURE_MAG_FILTER, mode);
        this._gl.texParameteri(this._gl.TEXTURE_2D, this._gl.TEXTURE_MIN_FILTER, mode);
        this._gl.bindTexture(this._gl.TEXTURE_2D, null);
    },


});
