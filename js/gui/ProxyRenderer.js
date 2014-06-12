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

        // --------------- Start of configuration section ----------------
        this._frameOutputCounter   = 0;
        this._frameMeasureCounter  = 0;
        this._frameOutputInterval  = 100;
        this._frameMeasureInterval = 10;

        this._pausePerFrameInMilliseconds = 10; // 100; // (Useful for GPU fans that we don't want to spin up too much... :-) )

        this._useBlending = false;

        // Number of proxy geometry splats (gl Points) in each direction, covering the viewport.
        // (Note that as long as glPoints are square, the ratio between these numbers should ideally equal the aspect ratio of the viewport.)
        this._splats_x = 0; // 16;
        this._splats_y = 0; // 16;
        // Overriding with slider!!!

        // This factor is just a guestimate at how much overlap we need between splats for those being moved toward the observer to fill in
        // gaps due to expansion caused by the perspective view, before new depth buffers arrive.
        this._splatOverlap = 0.5; // 1.0) splats are "shoulder to shoulder", 2.0) edge of one circular splat passes through center of neighbour to side or above/below
        // Overriding with slider!!!

        this._depthRingSize = 5;
        // this._coverageGridSize = 10;
        this._angleThreshold = (180.0/this._depthRingSize) / 180.0*3.1415926535; // Is this a sensible value? 180/#models degrees
        this._zoomThreshold = 1.1;

        // ---------------- End of configuration section -----------------

        this._frameTime = 0.0;
        this._lock = false; // for debugging
        this._lock2 = false; // for debugging

        this.gl = glContext;
        this.exposedModel = exposedModel;

        this.exposedModel.addLocalListener("reloadShader", dojo.hitch(this, function(event) {
            if(this.exposedModel.getElementValue("reloadShader")) {
                this._loadShaders();
            } else {
                this.exposedModel.updateElement("reloadShader", false);
            }
        }));

        this._loadShaders();

        // This solution amounts to a one-element cache/ring, and could be extended.
        this._proxyModelBeingProcessed = new gui.ProxyModel(this.gl);

//        this._proxyModelCoverage = new gui.ProxyModelCoverageGrid(this.gl, this._coverageGridSize);
        this._proxyModelCoverage = new gui.ProxyModelCoverageAngles(this.gl, this._depthRingSize, this._angleThreshold, this._zoomThreshold);

        var db = this.gl.getParameter(this.gl.DEPTH_BITS);
        console.log("Depth bits: " + db);
        console.log("ProxyRenderer constructor ended");
    },


   _loadShaders: function() {
       var oneIsLoaded = false;
       dojo.xhrGet( { url: "gui/autoProxy.fs",
                       handleAs: "text",
                       preventCache: true,
                       load: dojo.hitch(this, function(data, ioArgs) {
                           this._splat_fs_src = data;
                           if (oneIsLoaded) {
                               this._compileShaders();
                           } else {
                               oneIsLoaded = true;
                           }
                       })
                   } );
       dojo.xhrGet( { url: "gui/autoProxy.vs",
                       handleAs: "text",
                       preventCache: true,
                       load: dojo.hitch(this, function(data, ioArgs) {
                           this._splat_vs_src = data;
                           if (oneIsLoaded) {
                               this._compileShaders();
                           } else {
                               oneIsLoaded = true;
                           }
                       })
                   } );
   },


    _compileShaders: function() {
        console.log("Shader source should now have been read from files, compiling and linking program...");

        var available_extensions = this.gl.getSupportedExtensions();
        console.log("extensions: " + JSON.stringify(available_extensions));

        var frag_depth_ext = this.gl.getExtension("EXT_frag_depth");
        console.log("frag_depth_ext = " + frag_depth_ext);

        var splat_fs = this.gl.createShader(this.gl.FRAGMENT_SHADER);
        this.gl.shaderSource(splat_fs, this._splat_fs_src);
        this.gl.compileShader(splat_fs);
        if (!this.gl.getShaderParameter(splat_fs, this.gl.COMPILE_STATUS)) {
            alert("An error occurred compiling the splat_fs: " + this.gl.COMPILE_STATUS + ": " + this.gl.getShaderInfoLog(splat_fs));
            return null;
        }

        var splat_vs = this.gl.createShader(this.gl.VERTEX_SHADER);
        this.gl.shaderSource(splat_vs, this._splat_vs_src);
        this.gl.compileShader(splat_vs);
        if (!this.gl.getShaderParameter(splat_vs, this.gl.COMPILE_STATUS)) {
            alert("An error occurred compiling the splat_vs: " + this.gl.COMPILE_STATUS + ": " + this.gl.getShaderInfoLog(splat_vs));
            return null;
        }

        this._splatProgram = this.gl.createProgram();
        this.gl.attachShader(this._splatProgram, splat_vs);
        this.gl.attachShader(this._splatProgram, splat_fs);
        this.gl.linkProgram(this._splatProgram);
        if (!this.gl.getProgramParameter(this._splatProgram, this.gl.LINK_STATUS)) {
            alert("Unable to initialize the shader program: " + this.gl.LINK_STATUS + ": " + this.gl.getProgramInfoLog(this._splatProgram));
        }
    },


    setDepthData: function(imageAsText, depthBufferAsText, viewMatAsText, projMatAsText) {
        // console.log("setDepthData: Starting load process for image-as-text data");
        if (this._proxyModelBeingProcessed.state!=0) {
            console.log("A depth buffer is already in the pipeline, discarding the new one just received! (state=" + this._proxyModelBeingProcessed.state + ")");
        } else {
            if (!this._lock2) {
                // lock2 not set, we shall update the ring buffer as usual
                this._proxyModelBeingProcessed.setAll(depthBufferAsText, imageAsText, viewMatAsText, projMatAsText);
            }
            if (this._lock) {
                // All models have been reset and we just inserted a proxy model in the ring buffer, after this, we shall not insert any more, hence we set lock2.
                this._lock2 = true;
            }
        }
        // console.log("setDepthData: done");
    },


    _setUniform1i: function(prog, name) {
        if (this.gl.getUniformLocation(prog, name)) {
            this.gl.uniform1i( this.gl.getUniformLocation(prog, name), this.exposedModel.getElementValue(name) );
        }
    },


    // Adding a wrapper with an artificial pause in order to make sure the GPU fan doesn't spin up...
    render: function(matrices) {
        if ( this._pausePerFrameInMilliseconds > 0 ) {
            if (this._waitInProgress == true)
                return;
            this._matrices = matrices;
            this._waitInProgress = true;
            setTimeout( dojo.hitch(this, this.renderMain), this._pausePerFrameInMilliseconds);
        }
    },


    renderMain: function() {
        var matrices = this._matrices;

        if ( this._proxyModelBeingProcessed.state == 2 ) { // ... then there is a new proxy model that has been completely loaded, but not inserted into the ring.
            // this._proxyModelCoverage.processDepthDataReplaceOldest( this._proxyModelBeingProcessed );
            // this._proxyModelCoverage.processDepthDataReplaceOldestWhenDifferent( this._proxyModelBeingProcessed );
            // this._proxyModelCoverage.processDepthDataReplaceFarthestAway( this._proxyModelBeingProcessed );
            this._proxyModelCoverage.processDepthDataOptimizeCoverage(this._proxyModelBeingProcessed);
            this._proxyModelBeingProcessed = new gui.ProxyModel(this.gl);
        }

        if (this._splatProgram) {

            if ( this._useBlending) {
                this.gl.clearColor(0.2, 0.2, 0.2, 0.0);
                this.gl.enable(this.gl.BLEND);
                this.gl.blendFunc(this.gl.ONE, this.gl.ONE_MINUS_SRC_ALPHA); // s-factor, d-factor
            } else {
                this.gl.clearColor(0.2, 0.2, 0.2, 0.8);
            }
            this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);
            this.gl.enable(this.gl.DEPTH_TEST);

            this.gl.useProgram(this._splatProgram);
            var vertexPositionAttribute = this.gl.getAttribLocation( this._splatProgram, "aVertexPosition" );
            this.gl.enableVertexAttribArray( vertexPositionAttribute );

            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this._splatVertexBuffer);

            this._setUniform1i(this._splatProgram, "debugSplatCol");
            this._setUniform1i(this._splatProgram, "decayMode");
            this._setUniform1i(this._splatProgram, "roundSplats");
            this._setUniform1i(this._splatProgram, "screenSpaceSized");
            if ( this._useBlending) {
                if (this.gl.getUniformLocation(this._splatProgram, "useBlending")) {
                    this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "useBlending"), 1 );
                } else {
                    this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "useBlending"), 0 );
                }
            }
            this._setUniform1i(this._splatProgram, "mostRecentOffset");
            if (this.gl.getUniformLocation(this._splatProgram, "MV"))
                this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "MV"), false, matrices.m_from_world );
            if (this.gl.getUniformLocation(this._splatProgram, "PM"))
                this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "PM"), false, matrices.m_projection );
            this._setUniform1i(this._splatProgram, "splatSizeLimiting");
            this._setUniform1i(this._splatProgram, "ignoreIntraSplatTexCoo");
            this._setUniform1i(this._splatProgram, "splatOutline");
            if ( this.exposedModel.getElementValue("resetAllModels") ) {
                console.log("reset trykket");
                this.exposedModel.updateElement("resetAllModels", false);
                for (var i=0; i<this._depthRingSize; i++) {
                    this._proxyModelCoverage.proxyModelRing[i] = new gui.ProxyModel(this.gl);
                }
                this._lock = true;
                this._lock2 = false;
            }

            var ol = this.exposedModel.getElementValue("overlap") / 100.0;
            this._splatOverlap = ol;
            var splats = this.exposedModel.getElementValue("splats");
            if ( (this._splats_x!=splats) || (this._splats_y!=splats) ) {
                this._splats_x = splats;
                this._splats_y = splats;
                // We must (re-)initialize the vertex buffer!
                //this.gl.bindBuffer(this.gl.ARRAY_BUFFER, 0);
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
            }

            if (this.gl.getUniformLocation(this._splatProgram, "splats_x"))
                this.gl.uniform1f( this.gl.getUniformLocation(this._splatProgram, "splats_x"), this._splats_x );
            if (this.gl.getUniformLocation(this._splatProgram, "splats_y"))
                this.gl.uniform1f( this.gl.getUniformLocation(this._splatProgram, "splats_y"), this._splats_y );
            if (this.gl.getUniformLocation(this._splatProgram, "splatOverlap"))
                this.gl.uniform1f( this.gl.getUniformLocation(this._splatProgram, "splatOverlap"), this._splatOverlap );
            if (this.gl.getUniformLocation(this._splatProgram, "vp_width"))
                this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "vp_width"), this.gl.canvas.width );
            if (this.gl.getUniformLocation(this._splatProgram, "vp_height"))
                this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "vp_height"), this.gl.canvas.height );
            this.gl.vertexAttribPointer( vertexPositionAttribute, 2, this.gl.FLOAT, false, 0, 0);

            var t0 = performance.now();
            // Should combine these by putting the "most recent model" into the ring buffer...
            for (var i=0; i<this._depthRingSize; i++) {
                if (this._proxyModelCoverage.proxyModelRing[i].state==2) {
                    if (this.gl.getUniformLocation(this._splatProgram, "splatSetIndex")) {
                        this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "splatSetIndex"), i );
                    }
                    this.gl.activeTexture(this.gl.TEXTURE0);
                    this.gl.bindTexture(this.gl.TEXTURE_2D, this._proxyModelCoverage.proxyModelRing[i].depthTexture);
                    this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "depthImg"), 0 );
                    this.gl.activeTexture(this.gl.TEXTURE1);
                    this.gl.bindTexture(this.gl.TEXTURE_2D, this._proxyModelCoverage.proxyModelRing[i].rgbTexture);
                    this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "rgbImage"), 1 );
                    if (this.gl.getUniformLocation(this._splatProgram, "depthPMinv")) {
                        this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "depthPMinv"), false, this._proxyModelCoverage.proxyModelRing[i].projection_inverse );
                    }
                    if (this.gl.getUniformLocation(this._splatProgram, "depthMVinv")) {
                        this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "depthMVinv"), false, this._proxyModelCoverage.proxyModelRing[i].to_world );
                    }
                    if (this.gl.getUniformLocation(this._splatProgram, "projUnproj")) {
                        var A = mat4.create();
                        mat4.multiply( matrices.m_projection, matrices.m_from_world, A );
                        var B = mat4.create();
                        mat4.multiply( A, this._proxyModelCoverage.proxyModelRing[i].to_world, B);
                        mat4.multiply( B, this._proxyModelCoverage.proxyModelRing[i].projection_inverse, A );
                        this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "projUnproj"), false, A );
                    }
                    this.gl.drawArrays(this.gl.POINTS, 0, this._splats_x*this._splats_y);
                }
            } // end of loop over depth buffers
            if (this.exposedModel.getElementValue("alwaysShowMostRecent")) {
                if (this._proxyModelCoverage.mostRecentModel.state==2) {
                    if (this.gl.getUniformLocation(this._splatProgram, "splatSetIndex")) {
                        this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "splatSetIndex"), -1 );
                    }
                    this.gl.activeTexture(this.gl.TEXTURE0);
                    this.gl.bindTexture(this.gl.TEXTURE_2D, this._proxyModelCoverage.mostRecentModel.depthTexture);
                    this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "depthImg"), 0 );
                    this.gl.activeTexture(this.gl.TEXTURE1);
                    this.gl.bindTexture(this.gl.TEXTURE_2D, this._proxyModelCoverage.mostRecentModel.rgbTexture);
                    this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "rgbImage"), 1 );
                    if (this.gl.getUniformLocation(this._splatProgram, "depthPMinv")) {
                        this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "depthPMinv"), false, this._proxyModelCoverage.mostRecentModel.projection_inverse );
                    }
                    if (this.gl.getUniformLocation(this._splatProgram, "depthMVinv")) {
                        this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "depthMVinv"), false, this._proxyModelCoverage.mostRecentModel.to_world );
                    }
                    if (this.gl.getUniformLocation(this._splatProgram, "projUnproj")) {
                        var A = mat4.create();
                        mat4.multiply( matrices.m_projection, matrices.m_from_world, A );
                        var B = mat4.create();
                        mat4.multiply( A, this._proxyModelCoverage.mostRecentModel.to_world, B );
                        mat4.multiply( B, this._proxyModelCoverage.mostRecentModel.projection_inverse, A );
                        this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "projUnproj"), false, A );
                    }
                    this.gl.drawArrays(this.gl.POINTS, 0, this._splats_x*this._splats_y);
                }
            }

            // Timing-stuff
//            this.gl.flush();
//            if ( this._frameMeasureCounter < this._frameMeasureInterval ) {
//                this._frameTime += performance.now() - t0;
//            }
//            this._frameMeasureCounter++;
//            this._frameOutputCounter++;
//            if ( this._frameOutputCounter == this._frameOutputInterval ) {
//                console.log("Average shader time: " + (this._frameTime/this._frameMeasureInterval) + " (" + (1000.0/(this._frameTime/this._frameMeasureInterval)) + " fps)");
//                this._frameOutputCounter = 0;
//                this._frameMeasureCounter = 0;
//                this._frameTime = 0.0;
//            }

            if ( this._useBlending ) {
                this.gl.colorMask(false, false, false, true);
                this.gl.clearColor(1.0, 0.0, 0.0, 1.0);
                this.gl.clear(this.gl.COLOR_BUFFER_BIT);
                this.gl.colorMask(true, true, true, true);
                this.gl.disable(this.gl.BLEND);
            }

        }

        this._waitInProgress = false;
    }

});
