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

        this.gl = glContext;

        this.depthTexture = this.gl.createTexture();
        this.rgbTexture   = this.gl.createTexture();
        this.projection         = mat4.create();
        this.projection_inverse = mat4.create();
        this.from_world         = mat4.create();
        this.to_world           = mat4.create();

        // this.inUse        = false; // not needed?
        this._ready             = false;

        console.log("Constructor ended");
    },


    isReady: function() {
        return this._ready;
    },


    setAll: function(depthBufferAsText, imageAsText, viewMatAsText, projMatAsText) {
        this._ready   = false;
        var image = new Image();
        image.onload = dojo.hitch(this, function() {
            this.gl.bindTexture(this.gl.TEXTURE_2D, this.depthTexture);
            this.gl.texImage2D(this.gl.TEXTURE_2D, 0, this.gl.RGBA, this.gl.RGBA, this.gl.UNSIGNED_BYTE, image);
            this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MAG_FILTER, this.gl.NEAREST);
            this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.NEAREST); // _MIPMAP_NEAREST);
            //this.gl.generateMipmap(this.gl.TEXTURE_2D);
            this.gl.bindTexture(this.gl.TEXTURE_2D, null);

            var rgbImage = new Image();
            rgbImage.onload = dojo.hitch(this, function() {
                this.gl.bindTexture(this.gl.TEXTURE_2D, this.rgbTexture);
                this.gl.texImage2D(this.gl.TEXTURE_2D, 0, this.gl.RGBA, this.gl.RGBA, this.gl.UNSIGNED_BYTE, rgbImage);
                this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MAG_FILTER, this.gl.NEAREST);
                this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.NEAREST);
                //this.gl.generateMipmap(this.gl.TEXTURE_2D);
                this.gl.bindTexture(this.gl.TEXTURE_2D, null);

                this.projection         = projMatAsText.split(/ /);
                this.projection_inverse = mat4.inverse(mat4.create( this.projection ));
                this.from_world         = viewMatAsText.split(/ /);
                this.to_world           = mat4.inverse(mat4.create( this.from_world ));
                this._ready = true;
            });
            rgbImage.src = "data:image/png;base64," + imageAsText;

        });
        image.src = "data:image/png;base64," + depthBufferAsText;
    }


});
