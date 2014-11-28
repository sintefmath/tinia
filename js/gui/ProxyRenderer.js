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
dojo.require("gui.ProxyModelCoverageReplaceFarthestAway");
dojo.require("gui.ProxyModelCoverageReplaceOldest");
dojo.require("gui.ProxyModelCoverageReplaceOldestWhenDifferent");

dojo.declare("gui.ProxyRenderer", null, {


    // Will set the ProxyModelCoverage object to use for "algorithm <i>".
    _initProxyCoverage: function(i, glContext) {
        switch (i) {
            // What would be the proper way to do this? I.e., to avoid having to define these strings both in the client (here) and the server (the C++ job)?!
        case "0) AngleCoverage-5":
        case 0:
            this._proxyModelCoverage = new gui.ProxyModelCoverageAngles(glContext,
                                                                        5,                                  // Number of proxy models to keep
                                                                        (180.0/5) / 180.0*3.1415926535,     // Is this a sensible value? 180/#models degrees
                                                                        1.1);                               // "Zoom threshold"
            break;
        case "1) AngleCoverage-2":
        case 1:
            this._proxyModelCoverage = new gui.ProxyModelCoverageAngles(glContext,
                                                                        2,                                  // Number of proxy models to keep
                                                                        (180.0/2) / 180.0*3.1415926535,     // Is this a sensible value? 180/#models degrees
                                                                        1.1);                               // "Zoom threshold"
            break;
        case "2) OnlyMostRecent":
        case 2:
            this._proxyModelCoverage = new gui.ProxyModelCoverageAngles(glContext,
                                                                        0,                                  // Number of proxy models to keep
                                                                        (180.0/1) / 180.0*3.1415926535,     // Is this a sensible value? 180/#models degrees
                                                                        1.1);                               // "Zoom threshold"
            break;
        case "3) ReplaceOldestWhenDifferent-5":
        case 3:
            this._proxyModelCoverage = new gui.ProxyModelCoverageReplaceOldestWhenDifferent(glContext,
                                                                        5,                                  // Number of proxy models to keep
                                                                        (180.0/5) / 180.0*3.1415926535,     // Is this a sensible value? 180/#models degrees
                                                                        1.1);                               // "Zoom threshold"
            break;
        case "4) ReplaceOldest-5":
        case 4:
            this._proxyModelCoverage = new gui.ProxyModelCoverageReplaceOldest(glContext,
                                                                        5,                                  // Number of proxy models to keep
                                                                        (180.0/5) / 180.0*3.1415926535,     // Is this a sensible value? 180/#models degrees
                                                                        1.1);                               // "Zoom threshold"
            break;
        }
        console.log( "New proxy model replacement algo: " + i );
    },


    _setNumOfSplats: function(splats) {
        var splats = this.exposedModel.getElementValue("ap_splats");
        if ( (this._splats_x!=splats) || (this._splats_y!=splats) ) {
            this._splats_x = splats;
            this._splats_y = splats;
            // We must re-initialize the vertex buffer!
            this._splatVertexBuffer = this.gl.createBuffer();
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this._splatVertexBuffer);
            this._splatCoordinates = new Float32Array( this._splats_x*this._splats_y*2 );
            for (var i=0; i<this._splats_y; i++) {
                var v = (i+0.5)/this._splats_y;
                var ycoo = -1.0*(1.0-v) + 1.0*v;
                for (var j=0; j<this._splats_x; j++) {
                    var u = (j+0.5)/this._splats_x;
                    this._splatCoordinates[(this._splats_x*i+j)*2     ] = -1.0*(1.0-u) + 1.0*u;
                    this._splatCoordinates[(this._splats_x*i+j)*2 + 1 ] = ycoo;
                }
            }
            this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(this._splatCoordinates), this.gl.STATIC_DRAW);
        }
    },


    constructor: function(glContext, exposedModel, viewerKey) {


        // --------------- Start of configuration section ----------------

        // This is not yet implemented.
        this._useBlending = false;

        // The extension will not be attempted used if unavailable.
        this._useFragDepthExt = true;

        // Currently this is not set to any value used by the application
        this._backgroundCol = vec3.createFrom(0.0, 0.0, 0.0);

        // The most recent proxy model will always be shown, and with a small depth offset to force it in front
        this._alwaysShowMostRecent = true;

        // Number of proxy geometry splats (gl Points) in each direction, covering the viewport.
        // (Note that the ratio between these numbers should ideally equal the aspect ratio of the viewport for best results.)
        this._splats_x = 64;
        this._splats_y = 64;

        // This factor is just a guestimate at how much overlap we need between splats for those being moved toward the observer to fill in
        // gaps due to expansion caused by the perspective view, before new depth buffers arrive.
        this._splatOverlap = 2.0; // 1.0) splats are "shoulder to shoulder", 2.0) edge of one circular splat passes through center of neighbour to side or above/below

        // Proxy model replacement strategy
        // this._proxyModelCoverage = new gui.ProxyModelCoverageGrid(this.gl, this._coverageGridSize); // Not implemented yet
        this._initProxyCoverage(2, glContext);

        // --- For debugging, start
        this._frameOutputInterval         = 1000;
        this._frameMeasureInterval        = 100;
        this._pausePerFrameInMilliseconds = 0; // (100 is useful for GPU fans that we don't want to spin up too much... :-) )
        this._debugSplatCol               = 0;
        this._decayMode                   = 0;
        this._roundSplats                 = 0;
        this._screenSpaceSized            = 1;
        this._useISTC                     = 1;
        this._splatOutline                = 0;
        // --- For debugging, end

        // ---------------- End of configuration section -----------------

        this.gl = glContext;
        this.exposedModel = exposedModel;

        this._useFragDepthAndAvailable = false; // Will be set according to availability when we test for that

        this._debugging = (this.exposedModel.hasKey("ap_autoProxyDebugging")) && (this.exposedModel.getElementValue("ap_autoProxyDebugging"));
        if (this._debugging) {
            this._frameOutputCounter   = 0;
            this._frameMeasureCounter  = 0;
            this._frameTime = 0.0;
            this._totalTime = 0.0;
            this._t0 = 0.0;
            this._lock = false; // for debugging
            this._lock2 = false; // for debugging
        }

        this._splatVertexBuffer = this.gl.createBuffer();
        this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this._splatVertexBuffer);
        this._splatCoordinates = new Float32Array( this._splats_x*this._splats_y*2 );
        for (var i=0; i<this._splats_y; i++) {
            var v = (i+0.5)/this._splats_y;
            for (var j=0; j<this._splats_x; j++) {
                var u = (j+0.5)/this._splats_x;
                this._splatCoordinates[(this._splats_x*i+j)*2     ] = -1.0*(1.0-u) + 1.0*u;
                this._splatCoordinates[(this._splats_x*i+j)*2 + 1 ] = -1.0*(1.0-v) + 1.0*v;
            }
        }
        this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(this._splatCoordinates), this.gl.STATIC_DRAW);

        this._shaderSourceLoaded = false;
        this._loadShaders();

        // Setting up listeners for known configurable parameters. These are mainly for debugging and testing. (Meaning
        // that modification of defaults are for testing.) The application proxyCube sets up a GUI for manipulating
        // these.
        // 140911: Allowing these listeners even without debugging mode being enabled, so that applications may set their own values.
        //         Not adding a test for construction-time setting of all of these, just a subset.
        if (true) { // if (this._debugging) {
            //-------------------------------------------------------
            this.exposedModel.addLocalListener("ap_set_depth_size_128", dojo.hitch(this, function(event) {
                if(this.exposedModel.getElementValue("ap_set_depth_size_128")) {
                    this._loadShaders();
                    this.exposedModel.updateElement("ap_depthWidth", 128);
                    this.exposedModel.updateElement("ap_depthHeight", 128);
                } else {
                    this.exposedModel.updateElement("ap_set_depth_size_128", false);
                }
            }));
            //-------------------------------------------------------
            this.exposedModel.addLocalListener("ap_set_depth_size_256", dojo.hitch(this, function(event) {
                if(this.exposedModel.getElementValue("ap_set_depth_size_256")) {
                    this._loadShaders();
                    this.exposedModel.updateElement("ap_depthWidth", 256);
                    this.exposedModel.updateElement("ap_depthHeight", 256);
                } else {
                    this.exposedModel.updateElement("ap_set_depth_size_256", false);
                }
            }));
            //-------------------------------------------------------
            this.exposedModel.addLocalListener("ap_set_depth_size_512", dojo.hitch(this, function(event) {
                if(this.exposedModel.getElementValue("ap_set_depth_size_512")) {
                    this._loadShaders();
                    this.exposedModel.updateElement("ap_depthWidth", 512);
                    this.exposedModel.updateElement("ap_depthHeight", 512);
                } else {
                    this.exposedModel.updateElement("ap_set_depth_size_512", false);
                }
            }));

            //-------------------------------------------------------
            this.exposedModel.addLocalListener("ap_reloadShader", dojo.hitch(this, function(event) {
                if(this.exposedModel.getElementValue("ap_reloadShader")) {
                    this._loadShaders();
                } else {
                    this.exposedModel.updateElement("ap_reloadShader", false);
                }
            }));

            //-------------------------------------------------------
            if ( this.exposedModel.hasKey("ap_alwaysShowMostRecent") ) {
                this._alwaysShowMostRecent = this.exposedModel.getElementValue("ap_alwaysShowMostRecent") ? 1 : 0;
            }
            this.exposedModel.addLocalListener( "ap_alwaysShowMostRecent", dojo.hitch(this, function(event) {
                this._alwaysShowMostRecent = this.exposedModel.getElementValue("ap_alwaysShowMostRecent") ? 1 : 0;
            }) );

            //-------------------------------------------------------
            if ( this.exposedModel.hasKey("ap_overlap") ) {
                this._splatOverlap = this.exposedModel.getElementValue("ap_overlap") / 100.0;
            }
            this.exposedModel.addLocalListener( "ap_overlap", dojo.hitch(this, function(event) {
                this._splatOverlap = this.exposedModel.getElementValue("ap_overlap") / 100.0;
            }) );

            //-------------------------------------------------------
            if ( this.exposedModel.hasKey("ap_splats") ) {
                this._setNumOfSplats( this.exposedModel.getElementValue("ap_splats") );
            }
            this.exposedModel.addLocalListener( "ap_splats", dojo.hitch(this, function(event) {
                this._setNumOfSplats( this.exposedModel.getElementValue("ap_splats") );
            }) );

            //-------------------------------------------------------
            if ( this.exposedModel.hasKey("ap_debugSplatCol") ) {
                this._debugSplatCol = this.exposedModel.getElementValue("ap_debugSplatCol") ? 1 : 0;
                console.log("construction: ap_debugSplatCol = " + this._debugSplatCol);
            }
            this.exposedModel.addLocalListener( "ap_debugSplatCol", dojo.hitch(this, function(event) {
                this._debugSplatCol = this.exposedModel.getElementValue("ap_debugSplatCol") ? 1 : 0;
            }) );

            //-------------------------------------------------------
            this.exposedModel.addLocalListener( "ap_decayMode", dojo.hitch(this, function(event) {
                this._decayMode = this.exposedModel.getElementValue("ap_decayMode") ? 1: 0;
            }) );

            //-------------------------------------------------------
            this.exposedModel.addLocalListener( "ap_roundSplats", dojo.hitch(this, function(event) {
                this._roundSplats = this.exposedModel.getElementValue("ap_roundSplats") ? 1 : 0;
            }) );

            //-------------------------------------------------------
            if ( this.exposedModel.hasKey("ap_screenSpaceSized") ) {
                this._screenSpaceSized = this.exposedModel.getElementValue("ap_screenSpaceSized") ? 1 : 0;
            }
            this.exposedModel.addLocalListener( "ap_screenSpaceSized", dojo.hitch(this, function(event) {
                this._screenSpaceSized = this.exposedModel.getElementValue("ap_screenSpaceSized") ? 1 : 0;
            }) );

            //-------------------------------------------------------
            this.exposedModel.addLocalListener( "ap_useISTC", dojo.hitch(this, function(event) {
                this._useISTC = this.exposedModel.getElementValue("ap_useISTC") ? 1 : 0;
            }) );

            //-------------------------------------------------------
            if ( this.exposedModel.hasKey("ap_splatOutline") ) {
                this._splatOutline = this.exposedModel.getElementValue("ap_splatOutline") ? 1 : 0;
                console.log("ap_splatOutline=" + this._splatOutline);
            }
            this.exposedModel.addLocalListener( "ap_splatOutline", dojo.hitch(this, function(event) {
                this._splatOutline = this.exposedModel.getElementValue("ap_splatOutline") ? 1 : 0;
            }) );

            //-------------------------------------------------------
            if ( this.exposedModel.hasKey("ap_useFragExt") ) {
                this._useFragDepthExt = this.exposedModel.getElementValue("ap_useFragExt") ? 1 : 0;
                //console.log("xxxxxxxxxxxxxxxx compiling shaders because initial ap_useFragExt exists...");
                this._loadShaders(); // Must use this and not _compileShaders directly, since we cannot be sure that source has been loaded otherwise.
            }
            this.exposedModel.addLocalListener( "ap_useFragExt", dojo.hitch(this, function(event) {
                this._useFragDepthExt = this.exposedModel.getElementValue("ap_useFragExt") ? 1 : 0;
                this._loadShaders(); // Must use this and not _compileShaders directly, since we cannot be sure that source has been loaded otherwise.
            }) );

            //-------------------------------------------------------
            if ( this.exposedModel.hasKey("ap_mid_texel_sampling") ) {
                this._loadShaders();
            }
            this.exposedModel.addLocalListener( "ap_mid_texel_sampling", dojo.hitch(this, function(event) {
                this._loadShaders();
            }) );

            //-------------------------------------------------------
            if ( this.exposedModel.hasKey("ap_mid_splat_sampling") ) {
                this._loadShaders();
            }
            this.exposedModel.addLocalListener( "ap_mid_splat_sampling", dojo.hitch(this, function(event) {
                this._loadShaders();
            }) );

            //-------------------------------------------------------
            if ( this.exposedModel.hasKey("ap_small_delta_sampling") ) {
                this._loadShaders();
            }
            this.exposedModel.addLocalListener( "ap_small_delta_sampling", dojo.hitch(this, function(event) {
                this._loadShaders();
            }) );

            //-------------------------------------------------------
            if ( this.exposedModel.hasKey("ap_larger_delta_sampling") ) {
                this._loadShaders();
            }
            this.exposedModel.addLocalListener( "ap_larger_delta_sampling", dojo.hitch(this, function(event) {
                this._loadShaders();
            }) );

            //-------------------------------------------------------
            if ( this.exposedModel.hasKey("ap_autoProxyAlgo") ) {
                console.log("new algo: " + this.exposedModel.getElementValue("ap_autoProxyAlgo") );
                this._initProxyCoverage( this.exposedModel.getElementValue("ap_autoProxyAlgo"), this.gl );
            }
            this.exposedModel.addLocalListener( "ap_autoProxyAlgo", dojo.hitch(this, function(event) {
                //this._useFragDepthExt = this.exposedModel.getElementValue("ap_useFragExt") ? 1 : 0;
                //this._compileShaders();
                console.log("new algo: " + this.exposedModel.getElementValue("ap_autoProxyAlgo") );
                this._initProxyCoverage( this.exposedModel.getElementValue("ap_autoProxyAlgo"), this.gl );
            }) );

            // Here we should have a listener for backgroundCol, but does Tinia support the type "vec3"?
            //-------------------------------------------------------
        }

        // Listeners that are not for debugging only. Currently we use this only to clear the buffer in the event
        // that autoProxy has been enabled, but was then disabled. It could be that this is not needed if the clear-colours
        // used are set otherwise, for instance to the same value as the caller (Canvas-object) is using?!

        //-------------------------------------------------------
        if ( this.exposedModel.hasKey("ap_autoProxyDebugging") ) {
            this._debugging = this.exposedModel.getElementValue("ap_autoProxyDebugging");
            console.log("xxxxxxxxxxxxxxxx Recompiling shaders with/without DEBUG set...");
            this._loadShaders(); // Must use this and not _compileShaders directly, since we cannot be sure that source has been loaded otherwise.
        }
        this.exposedModel.addLocalListener( "ap_autoProxyDebugging", dojo.hitch(this, function(event) {
            this._debugging = this.exposedModel.getElementValue("ap_autoProxyDebugging");
            console.log("Recompiling shaders with/without DEBUG set...");
            this._loadShaders(); // Must use this and not _compileShaders directly, since we cannot be sure that source has been loaded otherwise.
        }) );

        //-------------------------------------------------------
        this.exposedModel.addLocalListener( "ap_useAutoProxy", dojo.hitch(this, function(event) {
            var tmp = this.exposedModel.getElementValue("ap_useAutoProxy");
            if (!tmp) {
                console.log("autoProxy was turned off. Clearing.");
                this._clearCanvas();
            }
        }) );

        this._proxyModelBeingProcessed = new gui.ProxyModel(this.gl);

        // console.log("ProxyRenderer constructor ended");
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
        console.log("******************** Shader source should now have been read from files, compiling and linking program...");

        var splat_vs_src = this._splat_vs_src;
        var splat_fs_src = this._splat_fs_src;

        if ( this._debugging ) {
            console.log("#DEBUG");
            splat_vs_src = "#define DEBUG\n" + splat_vs_src;
            splat_fs_src = "#define DEBUG\n" + splat_fs_src;
        } else {
            console.log("// #DEBUG");
        }

        var available_extensions = this.gl.getSupportedExtensions();
        console.log("extensions: " + JSON.stringify(available_extensions));

        this._frag_depth_ext = this.gl.getExtension("EXT_frag_depth");
        // Note that this is not a boolean, but an object is returned. Not sure if this object must be kept alive for the remainder
        // of the execution, but taking no chances.
        console.log("this._frag_depth_ext = " + this._frag_depth_ext);
        if (this.exposedModel.hasKey("ap_fragExtStatus")) {
            if (this._frag_depth_ext) {
                this.exposedModel.updateElement("ap_fragExtStatus", "(available)");
            } else {
                this.exposedModel.updateElement("ap_fragExtStatus", "(na)");
            }
        }
        if ( (this._frag_depth_ext) && (this._useFragDepthExt) ) {
            this._useFragDepthAndAvailable = true;
            splat_vs_src = "#define USE_FRAG_DEPTH_EXT\n" + splat_vs_src;
            splat_fs_src = "#extension GL_EXT_frag_depth : enable\n#define USE_FRAG_DEPTH_EXT\n" + splat_fs_src;
            console.log("#define USE_FRAG_DEPTH_EXT");
        } else {
            this._useFragDepthAndAvailable = false;
            console.log("// #define USE_FRAG_DEPTH_EXT");
        }

        if ( (this.exposedModel.hasKey("ap_mid_texel_sampling")) && (this.exposedModel.getElementValue("ap_mid_texel_sampling")) ) {
            splat_vs_src = "#define MID_TEXEL_SAMPLING\n" + splat_vs_src;
            splat_fs_src = "#define MID_TEXEL_SAMPLING\n" + splat_fs_src;
            console.log("#define MID_TEXEL_SAMPLING");
        } else {
            splat_vs_src = "// #define MID_TEXEL_SAMPLING\n" + splat_vs_src;
            splat_fs_src = "// #define MID_TEXEL_SAMPLING\n" + splat_fs_src;
            console.log("// #define MID_TEXEL_SAMPLING");
        }

        if ( (this.exposedModel.hasKey("ap_mid_splat_sampling")) && (this.exposedModel.getElementValue("ap_mid_splat_sampling")) ) {
            splat_vs_src = "#define MID_SPLAT_SAMPLING\n" + splat_vs_src;
            splat_fs_src = "#define MID_SPLAT_SAMPLING\n" + splat_fs_src;
            console.log("#define MID_SPLAT_SAMPLING");
        } else {
            splat_vs_src = "// #define MID_SPLAT_SAMPLING\n" + splat_vs_src;
            splat_fs_src = "// #define MID_SPLAT_SAMPLING\n" + splat_fs_src;
            console.log("// #define MID_SPLAT_SAMPLING");
        }

        if ( (this.exposedModel.hasKey("ap_small_delta_sampling")) && (this.exposedModel.getElementValue("ap_small_delta_sampling")) ) {
            splat_vs_src = "#define SMALL_DELTA_SAMPLING\n" + splat_vs_src;
            splat_fs_src = "#define SMALL_DELTA_SAMPLING\n" + splat_fs_src;
            console.log("#define SMALL_DELTA_SAMPLING");
        } else {
            splat_vs_src = "// #define SMALL_DELTA_SAMPLING\n" + splat_vs_src;
            splat_fs_src = "// #define SMALL_DELTA_SAMPLING\n" + splat_fs_src;
            console.log("// #define SMALL_DELTA_SAMPLING");
        }

        if ( (this.exposedModel.hasKey("ap_larger_delta_sampling")) && (this.exposedModel.getElementValue("ap_larger_delta_sampling")) ) {
            splat_vs_src = "#define LARGER_DELTA_SAMPLING\n" + splat_vs_src;
            splat_fs_src = "#define LARGER_DELTA_SAMPLING\n" + splat_fs_src;
            console.log("#define LARGER_DELTA_SAMPLING");
        } else {
            splat_vs_src = "// #define LARGER_DELTA_SAMPLING\n" + splat_vs_src;
            splat_fs_src = "// #define LARGER_DELTA_SAMPLING\n" + splat_fs_src;
            console.log("// #define LARGER_DELTA_SAMPLING");
        }

        var splat_fs = this.gl.createShader(this.gl.FRAGMENT_SHADER);
        this.gl.shaderSource(splat_fs, splat_fs_src);
        this.gl.compileShader(splat_fs);
        if (!this.gl.getShaderParameter(splat_fs, this.gl.COMPILE_STATUS)) {
            alert("An error occurred compiling the splat_fs: " + this.gl.COMPILE_STATUS + ": " + this.gl.getShaderInfoLog(splat_fs));
            console.log("FS source: ---------------------\n" + splat_fs_src + "\n----------------");
            return null;
        }

        var splat_vs = this.gl.createShader(this.gl.VERTEX_SHADER);
        this.gl.shaderSource(splat_vs, splat_vs_src);
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
            console.log("A depth buffer is already being processed, discarding the new one just received! (state=" + this._proxyModelBeingProcessed.state + ")");
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


    // Some useful wrappers
    _setUniform1i: function(prog, name, value) {
        if (this.gl.getUniformLocation(prog, name)) {
            this.gl.uniform1i( this.gl.getUniformLocation(prog, name), value );
        }
    },
    _setUniform1f: function(prog, name, value) {
         if (this.gl.getUniformLocation(prog, name)) {
             this.gl.uniform1f( this.gl.getUniformLocation(prog, name), value );
         }
     },
    _setUniform3fv: function(prog, name, value) {
         if (this.gl.getUniformLocation(prog, name)) {
             this.gl.uniform3fv( this.gl.getUniformLocation(prog, name), value );
         }
     },
    _setUniformMatrix4fv: function(prog, name, value) {
        if (this.gl.getUniformLocation(prog, name)) {
            this.gl.uniformMatrix4fv( this.gl.getUniformLocation(prog, name), false, value );
        }
    },


    // Adding a wrapper with an artificial pause in order to make sure the GPU fan doesn't spin up...
    render: function(matrices) {
        if ( (this._debugging) && (this._pausePerFrameInMilliseconds>0) ) {
            if (this._waitInProgress == true)
                return;
            this._matrices = matrices;
            this._waitInProgress = true;
            setTimeout( dojo.hitch(this, this.renderMain), this._pausePerFrameInMilliseconds);
        } else {
            this._matrices = matrices;
            this.renderMain();
        }
    },


    _clearCanvas: function() {
        if ( this.exposedModel.getElementValue("ap_useAutoProxy") ) {

            if ( this._useBlending) {
                if (this._debugging) {
                    this.gl.clearColor(0.2, 0.2, 0.2, 0.0);
                } else {
                    this.gl.clearColor(this._backgroundCol[0], this._backgroundCol[1], this._backgroundCol[2], 0.0);
                }
                this.gl.enable(this.gl.BLEND);
                this.gl.blendFunc(this.gl.ONE, this.gl.ONE_MINUS_SRC_ALPHA); // s-factor, d-factor
            } else {
                if (this._debugging) {
                    // this.gl.clearColor(0.2, 0.2, 0.2, 0.8); // Use something smaller than 1.0 (0.8 for instance) for alpha to see the "ghost images"
                    this.gl.clearColor(0.0, 0.2, 0.0, 1.0); // Use something smaller than 1.0 (0.8 for instance) for alpha to see the "ghost images"
                } else {
                    this.gl.clearColor(this._backgroundCol[0], this._backgroundCol[1], this._backgroundCol[2], 1.0);
                }
            }
            this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);

        } else {
            // Not using the auto-proxy. Why do we end up in _clearCanvas at all, then?!
            this.gl.clearColor(this._backgroundCol[0], this._backgroundCol[1], this._backgroundCol[2], 0.0);
            this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);
        }
    },


    renderMain: function() {
        if ( !this.exposedModel.getElementValue("ap_useAutoProxy") )
            return; // Just to be sure we don't mess up anything after this has been turned off

        var matrices = this._matrices;

        if ( this._proxyModelBeingProcessed.state == 2 ) { // ... then there is a new proxy model that has been completely loaded, but not inserted into the ring.
            this._proxyModelCoverage.processDepthData( this._proxyModelBeingProcessed );
            this._proxyModelBeingProcessed = new gui.ProxyModel(this.gl);
        }

        if (this._splatProgram) {

            this._clearCanvas();
            this.gl.enable(this.gl.DEPTH_TEST);

            this.gl.useProgram(this._splatProgram);
            var vertexPositionAttribute = this.gl.getAttribLocation( this._splatProgram, "aVertexPosition" );
            this.gl.enableVertexAttribArray( vertexPositionAttribute );
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this._splatVertexBuffer);
            this._setUniform1i(this._splatProgram, "debugSplatCol", this._debugSplatCol);
            this._setUniform1i(this._splatProgram, "decayMode", this._decayMode);
            this._setUniform1i(this._splatProgram, "roundSplats", this._roundSplats);
            this._setUniform1i(this._splatProgram, "screenSpaceSized", this._screenSpaceSized);
            this._setUniform1i(this._splatProgram, "useISTC", this._useISTC);
            this._setUniform1i(this._splatProgram, "splatOutline", this._splatOutline);
            this._setUniform1i(this._splatProgram, "useBlending", this._useBlending ? 1 : 0);
//            this._setUniformMatrix4fv(this._splatProgram, "MV", matrices.m_from_world);
//            this._setUniformMatrix4fv(this._splatProgram, "PM", matrices.m_projection);
            if ( (this._debugging) && (this.exposedModel.hasKey("ap_resetAllModels")) && (this.exposedModel.getElementValue("ap_resetAllModels")) ) {
                this.exposedModel.updateElement("ap_resetAllModels", false);
                for (var i=0; i<this._proxyModelCoverage.bufferRingSize; i++) {
                    this._proxyModelCoverage.proxyModelRing[i] = new gui.ProxyModel(this.gl);
                }
                this._lock = true;
                this._lock2 = false;
            }
            this._setUniform1f(this._splatProgram, "splats_x", this._splats_x);
            this._setUniform1f(this._splatProgram, "splats_y", this._splats_y);
            this._setUniform1f(this._splatProgram, "splatOverlap", this._splatOverlap);
            this._setUniform1i(this._splatProgram, "vp_width", this.gl.canvas.width);
            this._setUniform1i(this._splatProgram, "vp_height", this.gl.canvas.height);
            this._setUniform3fv(this._splatProgram, "backgroundCol", this._backgroundCol);
            this.gl.vertexAttribPointer( vertexPositionAttribute, 2, this.gl.FLOAT, false, 0, 0);

            this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "depthImg"), 0 );
            this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "rgbImage"), 1 );

            var t0 = performance.now();
            for (var i=0; i<this._proxyModelCoverage.bufferRingSize; i++) {
                if (this._proxyModelCoverage.proxyModelRing[i].state==2) {
                    this._setUniform1i(this._splatProgram, "splatSetIndex", i);
                    this.gl.activeTexture(this.gl.TEXTURE0);
                    this.gl.bindTexture(this.gl.TEXTURE_2D, this._proxyModelCoverage.proxyModelRing[i].depthTexture);
                    this.gl.activeTexture(this.gl.TEXTURE1);
                    this.gl.bindTexture(this.gl.TEXTURE_2D, this._proxyModelCoverage.proxyModelRing[i].rgbTexture);
//                    this._setUniformMatrix4fv(this._splatProgram, "depthPMinv", this._proxyModelCoverage.proxyModelRing[i].projection_inverse);
//                    this._setUniformMatrix4fv(this._splatProgram, "depthMVinv", this._proxyModelCoverage.proxyModelRing[i].to_world);
                    if (this.gl.getUniformLocation(this._splatProgram, "projUnproj")) {
                        // Could this be simplified?
                        var A = mat4.create();
                        mat4.multiply( matrices.m_projection, matrices.m_from_world, A );
                        var B = mat4.create();
                        mat4.multiply( A, this._proxyModelCoverage.proxyModelRing[i].to_world, B);
                        mat4.multiply( B, this._proxyModelCoverage.proxyModelRing[i].projection_inverse, A );
                        this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "projUnproj"), false, A );
                    }
                    if (this.gl.getUniformLocation(this._splatProgram, "camDir")) {
                        var A = mat4.create();
                        mat4.multiply( matrices.m_from_world, this._proxyModelCoverage.proxyModelRing[i].to_world, A ); // MV * MVinv[i]
                        var camDir = vec3.create( [-A[8], -A[9], -A[10]] );
                        vec3.normalize(camDir);
                        this.gl.uniform3fv( this.gl.getUniformLocation(this._splatProgram, "camDir"), camDir );
                    }
                    this.gl.drawArrays(this.gl.POINTS, 0, this._splats_x*this._splats_y);
                }
            }
            // Currently, the "most recent model" looks awful if we use it without an offset to get it in front, and it looks awful with offset, *if* we
            // don't have the FragDepthExtension, so in this latter case, we disable it altogether.
            if ( (this._alwaysShowMostRecent) ) { // && (this._useFragDepthAndAvailable) ) {
                if (this._proxyModelCoverage.mostRecentModel.state==2) {
                    this._setUniform1i(this._splatProgram, "splatSetIndex", -1);
                    this.gl.activeTexture(this.gl.TEXTURE0);
                    this.gl.bindTexture(this.gl.TEXTURE_2D, this._proxyModelCoverage.mostRecentModel.depthTexture);
                    this.gl.activeTexture(this.gl.TEXTURE1);
                    this.gl.bindTexture(this.gl.TEXTURE_2D, this._proxyModelCoverage.mostRecentModel.rgbTexture);
//                    this._setUniformMatrix4fv(this._splatProgram, "depthPMinv", this._proxyModelCoverage.mostRecentModel.projection_inverse);
//                    this._setUniformMatrix4fv(this._splatProgram, "depthMVinv", this._proxyModelCoverage.mostRecentModel.to_world);
                    if (this.gl.getUniformLocation(this._splatProgram, "projUnproj")) {
                        // Could this be simplified?
                        var A = mat4.create();
                        mat4.multiply( matrices.m_projection, matrices.m_from_world, A );
                        var B = mat4.create();
                        mat4.multiply( A, this._proxyModelCoverage.mostRecentModel.to_world, B );
                        mat4.multiply( B, this._proxyModelCoverage.mostRecentModel.projection_inverse, A );
                        this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "projUnproj"), false, A );
                    } else {
                        console.log("Huh?! Should not happen...");
                    }
                    if (this.gl.getUniformLocation(this._splatProgram, "camDir")) {
                        var A = mat4.create();
                        mat4.multiply( matrices.m_from_world, this._proxyModelCoverage.mostRecentModel.to_world, A ); // MV * MVinv[mostRecentModel]
                        var camDir = vec3.create( [-A[8], -A[9], -A[10]] );
                        vec3.normalize(camDir);
                        this.gl.uniform3fv( this.gl.getUniformLocation(this._splatProgram, "camDir"), camDir );
                    }
                    this.gl.drawArrays(this.gl.POINTS, 0, this._splats_x*this._splats_y);
                }
            }

            // Timing-stuff
            //            if (this._debugging) {
            if (false) {
                //            this.gl.flush();
                if ( this._frameMeasureCounter < this._frameMeasureInterval ) {
                    var t1 = performance.now();
                    this._frameTime += t1 - t0;
                    this._totalTime += t1 - this._t0;
                    this._t0 = t1;
                }
                this._frameMeasureCounter++;
                this._frameOutputCounter++;
                if ( this._frameOutputCounter == this._frameOutputInterval ) {
                    var ms = Math.floor(this._frameTime/this._frameMeasureInterval*100.0)/100.0;
                    var fps = Math.floor(1000.0/(this._frameTime/this._frameMeasureInterval));
                    var ms_total = Math.floor(this._totalTime/this._frameMeasureInterval*100.0)/100.0;
                    var fps_total = Math.floor(1000.0/(this._totalTime/this._frameMeasureInterval));
                    console.log("Average shader time: " + (ms) + " (" + (fps) + " fps), total time: " + (ms_total) + " (" + (fps_total) + " fps)");
                    if (this.exposedModel.hasKey("ap_consoleLog")) {
                        this.exposedModel.updateElement("ap_consoleLog", "Average shader time: " + (ms) + " (" + (fps) + " fps), total time: " + (ms_total) + " (" + (fps_total) + " fps)");
                    }
                    this._frameOutputCounter = 0;
                    this._frameMeasureCounter = 0;
                    this._frameTime = 0.0;
                    this._totalTime = 0.0;
                }
                this.exposedModel.updateElement("ap_cntr", this.exposedModel.getElementValue("ap_cntr") + 1 );
            }

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
