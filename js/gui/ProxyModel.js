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

        this.inUse        = false; // not needed?
        this.setNotReady();

        console.log("Constructor ended");
    },


    setDepthBuffer: function(depthBufferAsText) {
        //console.log("setDepthBuffer: Setting buffer, count = " + this._depthBufferCounter);
        var image = new Image();
        image.onload = dojo.hitch(this, function() {
            this.gl.bindTexture(this.gl.TEXTURE_2D, this.depthTexture);
            this.gl.texImage2D(this.gl.TEXTURE_2D, 0, this.gl.RGBA, this.gl.RGBA, this.gl.UNSIGNED_BYTE, image);
            this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MAG_FILTER, this.gl.NEAREST);
            this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.NEAREST); // _MIPMAP_NEAREST);
            //this.gl.generateMipmap(this.gl.TEXTURE_2D);
            this.gl.bindTexture(this.gl.TEXTURE_2D, null);
            //console.log("Updated texture (depth buffer)");
            this._depthSet = true;
        });
        image.src = "data:image/png;base64," + depthBufferAsText;
        // console.log("Depth buffer set");
    },


    setRGBimage: function(imageAsText) {
        // console.log("setRGBimage: Setting image, count = " + this._depthBufferCounter);
        var image = new Image();
        image.onload = dojo.hitch(this, function() {
            this.gl.bindTexture(this.gl.TEXTURE_2D, this.rgbTexture);
            this.gl.texImage2D(this.gl.TEXTURE_2D, 0, this.gl.RGBA, this.gl.RGBA, this.gl.UNSIGNED_BYTE, image);
            this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MAG_FILTER, this.gl.NEAREST);
            this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.NEAREST);
            //this.gl.generateMipmap(this.gl.TEXTURE_2D);
            this.gl.bindTexture(this.gl.TEXTURE_2D, null);
            //console.log("Updated texture (image)");
            this._rgbSet = true;
        });
        image.src = "data:image/png;base64," + imageAsText;
        // console.log("Image set");
    },


    setMatrices: function( viewMatAsText, projMatAsText ) {
//        console.log("proxyModel::setViewMat: Setting view matrix from server");
//        console.log("              view mat: " + viewMatAsText);
//        console.log("              proj mat: " + projMatAsText);
        this.projection         = projMatAsText.split(/ /);
        this.projection_inverse = mat4.inverse(mat4.create( this.projection ));
        this.from_world         = viewMatAsText.split(/ /);
        this.to_world           = mat4.inverse(mat4.create( this.from_world ));
        this._matSet = true;
    },


    // This is required because ProxyModel use different set*-methods that the caller cannot know the order in which are completed.
    // By calling this method, all states are invalidated, so that it can be tested for readiness, meaning that all set*-methods have
    // completed.
    setNotReady: function() {
        this._depthSet = false;
        this._rgbSet   = false;
        this._matSet   = false;
    },


    isReady: function() {
        return this._depthSet && this._rgbSet && this._matSet;
    }



});
