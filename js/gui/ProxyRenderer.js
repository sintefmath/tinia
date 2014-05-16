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

dojo.require("gui.ProxyModel");
dojo.require("gui.ProxyModelCoverageGrid");
dojo.require("gui.ProxyModelCoverageAngles");

dojo.declare("gui.ProxyRenderer", null, {


    constructor: function(glContext, exposedModel, viewerKey) {

        // Number of proxy geometry splats (gl Points) in each direction, covering the viewport.
        // (Note that as long as glPoints are square, the ratio between these numbers should ideally equal the aspect ratio of the viewport.)
        this._splats_x = 16;
        this._splats_y = 16;

        // This factor is just a guestimate at how much overlap we need between splats for those being moved toward the observer to fill in
        // gaps due to expansion caused by the perspective view, before new depth buffers arrive.
        this._splatOverlap = 0.5; // 1.0) splats are "shoulder to shoulder", 2.0) edge of one circular splat passes through center of neighbour to side or above/below

        this._depthRingSize = 4;
        this._coverageGridSize = 10;
        this._angleThreshold = (180.0/this._depthRingSize) / 180.0*3.1415926535; // Is this a sensible value? 180/#models degrees
        this._zoomThreshold = 1.1;

        // ---------------- End of configuration section -----------------

        this.gl = glContext;
        this.exposedModel = exposedModel;

        dojo.xhrGet( { url: "gui/autoProxy.fs",
                        handleAs: "text",
                        load: dojo.hitch(this, function(data, ioArgs) {
                            this._splat_fs_src = data;
                        })
                    } );
        dojo.xhrGet( { url: "gui/autoProxy.vs",
                        handleAs: "text",
                        load: dojo.hitch(this, function(data, ioArgs) {
                            this._splat_vs_src = data;
                        })
                    } );

        // This solution amounts to a one-element cache/ring, and could be extended.
        this._proxyModelBeingProcessed = new gui.ProxyModel(this.gl);

//        this._proxyModelCoverage = new gui.ProxyModelCoverageGrid(this.gl, this._coverageGridSize);
        this._proxyModelCoverage = new gui.ProxyModelCoverageAngles(this.gl, this._depthRingSize, this._angleThreshold, this._zoomThreshold);

        this._splatVertexBuffer = this.gl.createBuffer();
        this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this._splatVertexBuffer);
        this._splatCoordinates = new Float32Array( this._splats_x*this._splats_y*2 );
        for (var i=0; i<this._splats_y; i++) {
            for (var j=0; j<this._splats_x; j++) {
                var u = (j+0.5)/this._splats_x;
                var v = (i+0.5)/this._splats_y;
                this._splatCoordinates[(this._splats_x*i+j)*2     ] = -1.0*(1.0-u) + 1.0*u;
                this._splatCoordinates[(this._splats_x*i+j)*2 + 1 ] = -1.0*(1.0-v) + 1.0*v;
            }
        }
        this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(this._splatCoordinates), this.gl.STATIC_DRAW);

        console.log("ProxyRenderer constructor ended");
    },


    compileShaders: function() {
        console.log("Shader source should now have been read from files, compiling and linking program...");

        var splat_fs = this.gl.createShader(this.gl.FRAGMENT_SHADER);
        this.gl.shaderSource(splat_fs, this._splat_fs_src);
        this.gl.compileShader(splat_fs);
        if (!this.gl.getShaderParameter(splat_fs, this.gl.COMPILE_STATUS)) {
            alert("An error occurred compiling the splat_fs: " + this.gl.getShaderInfoLog(splat_fs));
            return null;
        }

        var splat_vs = this.gl.createShader(this.gl.VERTEX_SHADER);
        this.gl.shaderSource(splat_vs, this._splat_vs_src);
        this.gl.compileShader(splat_vs);
        if (!this.gl.getShaderParameter(splat_vs, this.gl.COMPILE_STATUS)) {
            alert("An error occurred compiling the splat_vs: " + this.gl.getShaderInfoLog(splat_vs));
            return null;
        }

        this._splatProgram = this.gl.createProgram();
        this.gl.attachShader(this._splatProgram, splat_vs);
        this.gl.attachShader(this._splatProgram, splat_fs);
        this.gl.linkProgram(this._splatProgram);
        if (!this.gl.getProgramParameter(this._splatProgram, this.gl.LINK_STATUS)) {
            alert("Unable to initialize the shader program. (gl.LINK_STATUS not ok,)");
        }
    },


    setDepthData: function(imageAsText, depthBufferAsText, viewMatAsText, projMatAsText) {
        // console.log("setDepthData: Starting load process for image-as-text data");
        this._proxyModelBeingProcessed.setAll(depthBufferAsText, imageAsText, viewMatAsText, projMatAsText);
    },


    render: function(matrices) {

        if ( (!this._splatProgram) && (this._splat_vs_src) && (this._splat_fs_src) ) {
            this.compileShaders();
        }

        if ( this._proxyModelBeingProcessed.state == 2 ) { // ... then there is a new proxy model that has been completely loaded, but not inserted into the ring.
            // this._proxyModelCoverage.processDepthDataReplaceOldest( this._proxyModelBeingProcessed );
            // this._proxyModelCoverage.processDepthDataReplaceOldestWhenDifferent( this._proxyModelBeingProcessed );
            // this._proxyModelCoverage.processDepthDataReplaceFarthestAway( this._proxyModelBeingProcessed );
            this._proxyModelCoverage.processDepthDataOptimizeCoverage(this._proxyModelBeingProcessed);
            this._proxyModelBeingProcessed = new gui.ProxyModel(this.gl);
        }

        if (this._splatProgram) {

            this.gl.clearColor(0.2, 0.2, 0.2, 1.0);
            this.gl.clearColor(0.2, 0.2, 0.2, 0.8);
            this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);

            // Strange... Blending disabled. Clearing done before proxy rendering. Then, still, ...:
            //
            // (Two different effects are observed with some settings: 1. snapshot is lingering behind proxy even when glClear is done,
            // and 2. snapshot residuals are observed in cleared parts and redrawn (by proxy) parts.)
            //
            // clearColor A     Proxy rendered A    Result
            // ------------------------------------------------------------------------------------
            // 0                0                   snapShot lingers in un-re-rendered parts. (Seems less saturated, as if distd with 0.5 perhaps)
            //                                      New proxy seems to be blended with old image, get saturation in some places.
            //
            // 0                0.5                 snapShot lingers in un-re-rendered parts.
            //                                      Also lingers in re-rendered parts! As if blending was enabled.
            //                                      In other words, both "effects" at the same time.
            //
            // 0                1                   snapShot lingers in un-re-rendered (by proxy geometry) parts of the image.
            //                                      But the background color specified is used around the snapShot fragments actually set, even though the
            //                                      snapShot itself should have another background (black)!
            //
            // 1                0                   snapShot does not linger in un-re-rendered parts, here the clear-color is used
            //                                      But; lingers in re-rendered parts! As if blending was enabled.
            //                                      But if any sort of blending is being done, why does not the snapShot linger in un-re-rendered parts?!
            //                                      (Why does the clear remove old snapshot from un-rendered parts but not those that are overdrawn with new proxy fragments?!)
            //
            // 1                0.5                 Same as for proxy-alpha=1, but we get saturation in lesser extent (snapshot + proxy < 1 e.g. for snap-red + proxy-cyan)
            //                                                                                                                                      (1, 0, 0)  + 0.5*(0, 1, 1) = (1, 0.5, 0.5)
            //                                                                                                                      instead of      (1, 0, 0)  + 1.0*(0, 1, 1) = (1, 1, 1 ) it looks like...
            //
            // 1                1                   More in line with expectations: no lingering snapShot at all. Background cleared to selected color everywhere.


            //            this.gl.enable(this.gl.BLEND);
            this.gl.disable(this.gl.BLEND);

//            this.gl.blendFunc(this.gl.SRC_ALPHA, this.gl.SRC_ALPHA); // s-factor, d-factor
//            this.gl.disable(this.gl.DEPTH_TEST);


            this.gl.useProgram(this._splatProgram);
            var vertexPositionAttribute = this.gl.getAttribLocation( this._splatProgram, "aVertexPosition" );
            this.gl.enableVertexAttribArray( vertexPositionAttribute );

            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this._splatVertexBuffer);

            var debugmode = this.exposedModel.getElementValue("debugmode");
            if (this.gl.getUniformLocation(this._splatProgram, "debugSplatCol")) {
                this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "debugSplatCol"), debugmode );
            }
            var decayMode = this.exposedModel.getElementValue("decaymode");
            if (this.gl.getUniformLocation(this._splatProgram, "decayMode")) {
                this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "decayMode"), decayMode );
            }
            var pieSplats = this.exposedModel.getElementValue("piesplats");
            if (this.gl.getUniformLocation(this._splatProgram, "pieSplats")) {
                this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "pieSplats"), pieSplats );
            }
            if (this.gl.getUniformLocation(this._splatProgram, "MV")) {
                this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "MV"), false, matrices.m_from_world );
            }
            if (this.gl.getUniformLocation(this._splatProgram, "PM")) {
                this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "PM"), false, matrices.m_projection );
            }
            if (this.gl.getUniformLocation(this._splatProgram, "splatSize")) {
                var splatSizeX = this.gl.canvas.width  / this._splats_x;
                var splatSizeY = this.gl.canvas.height / this._splats_y;
                var splatSize = Math.max(splatSizeX, splatSizeY) * this._splatOverlap;
                if ( Math.abs(splatSizeX-splatSizeY) > 0.001 ) {
                    console.log("Viewport size and number of splats indicate that splats with non-unity aspect ratio has been requested!" +
                                " x) " + splatSizeX + " y) " + splatSizeY + " used) " +splatSize);
                }
                this.gl.uniform1f( this.gl.getUniformLocation(this._splatProgram, "splatSize"), splatSize );
            }
            if (this.gl.getUniformLocation(this._splatProgram, "splats_x")) {
                this.gl.uniform1f( this.gl.getUniformLocation(this._splatProgram, "splats_x"), this._splats_x );
            }
            if (this.gl.getUniformLocation(this._splatProgram, "splats_y")) {
                this.gl.uniform1f( this.gl.getUniformLocation(this._splatProgram, "splats_y"), this._splats_y );
            }
            if (this.gl.getUniformLocation(this._splatProgram, "splatOverlap")) {
                this.gl.uniform1f( this.gl.getUniformLocation(this._splatProgram, "splatOverlap"), this._splatOverlap );
            }
            this.gl.vertexAttribPointer( vertexPositionAttribute, 2, this.gl.FLOAT, false, 0, 0);

            for (var i=0; i<this._depthRingSize; i++) {
                if (this._proxyModelCoverage.proxyModelRing[i].state==2) {
                    if (this.gl.getUniformLocation(this._splatProgram, "splatSetIndex")) {
                        this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "splatSetIndex"), i );
                    }
                    this.gl.activeTexture(this.gl.TEXTURE0);
                    this.gl.bindTexture(this.gl.TEXTURE_2D, this._proxyModelCoverage.proxyModelRing[i].depthTexture);
                    this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "uSampler"), 0 );
                    this.gl.activeTexture(this.gl.TEXTURE1);
                    this.gl.bindTexture(this.gl.TEXTURE_2D, this._proxyModelCoverage.proxyModelRing[i].rgbTexture);
                    this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "rgbImage"), 1 );
                    if (this.gl.getUniformLocation(this._splatProgram, "depthPMinv")) {
                        this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "depthPMinv"), false, this._proxyModelCoverage.proxyModelRing[i].projection_inverse );
                    }
                    if (this.gl.getUniformLocation(this._splatProgram, "depthMVinv")) {
                        this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "depthMVinv"), false, this._proxyModelCoverage.proxyModelRing[i].to_world );
                    }
                    this.gl.drawArrays(this.gl.POINTS, 0, this._splats_x*this._splats_y);
                }
            } // end of loop over depth buffers

//            this.gl.colorMask(this.gl.FALSE, this.gl.FALSE, this.gl.FALSE, this.gl.TRUE);
//            this.gl.clearColor(0.5, 0.0, 0.0, 0.5);
//            this.gl.clear(this.gl.COLOR_BUFFER_BIT);
//            this.gl.colorMask(this.gl.TRUE, this.gl.TRUE, this.gl.TRUE, this.gl.TRUE);

        }
        // console.log("rendering");
    }

});
