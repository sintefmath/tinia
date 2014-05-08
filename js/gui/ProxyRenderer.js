dojo.declare("gui.ProxyRenderer", null, {


    constructor: function(glContext, exposedModel, viewerKey) {

        this._depthBufferCounter = 0;
        this._subscriptionCounter = 0;

        // Number of proxy geometry splats (gl Points) in each direction, covering the viewport.
        // (Note that as long as glPoints are square, the ratio between these numbers should ideally equal the aspect ratio of the viewport.)
        this._splats_x = 16;
        this._splats_y = 16;

        // This factor is just a guestimate at how much overlap we need between splats for those being moved toward the observer to fill in
        // gaps due to expansion caused by the perspective view, before new depth buffers arrive.
        this._splatOverlap = 2.0; // 1.4;


        this.gl = glContext;

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

        dojo.subscribe("/model/updateSendStart", dojo.hitch(this, function(xml) {
            this._subscriptionCounter++;
            console.log("Subscriber: Setting matrices, _subscriptionCounter: " + this._subscriptionCounter);
            var viewer = exposedModel.getElementValue(viewerKey);
            // These are matrices available when we set the depth texture. But are these the exact
            // matrices used by the server to produce the depth image? Or do they just coincide
            // more or less in time?
            this._depth_matrices = {
                m_projection:         viewer.getElementValue("projection"),
                m_projection_inverse: mat4.inverse(mat4.create(viewer.getElementValue("projection"))),
                m_to_world:           mat4.inverse(mat4.create(viewer.getElementValue("modelview"))),
                m_from_world:         viewer.getElementValue("modelview")
            }
        }));

        this.depthTexture = this.gl.createTexture();
        this.rgbTexture = this.gl.createTexture();

        this._splatVertexBuffer = this.gl.createBuffer();
        this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this._splatVertexBuffer);
        this._splatCoordinates = new Float32Array( this._splats_x*this._splats_y*2 );
        for (i=0; i<this._splats_y; i++) {
            for (j=0; j<this._splats_x; j++) {
                var u = (j+0.5)/this._splats_x;
                var v = (i+0.5)/this._splats_y;
                this._splatCoordinates[(this._splats_x*i+j)*2     ] = -1.0*(1.0-u) + 1.0*u;
                this._splatCoordinates[(this._splats_x*i+j)*2 + 1 ] = -1.0*(1.0-v) + 1.0*v;
            }
        }
        this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(this._splatCoordinates), this.gl.STATIC_DRAW);

        console.log("Constructor ended");
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


    setDepthBuffer: function(depthBufferAsText) {
        this._depthBufferCounter++;
        console.log("setDepthBuffer: Setting buffer, count = " + this._depthBufferCounter);
        var image = new Image();
        image.onload = dojo.hitch(this, function() {
            this.gl.bindTexture(this.gl.TEXTURE_2D, this.depthTexture);
            this.gl.texImage2D(this.gl.TEXTURE_2D, 0, this.gl.RGBA, this.gl.RGBA, this.gl.UNSIGNED_BYTE, image);
            this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MAG_FILTER, this.gl.NEAREST);
            this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.NEAREST); // _MIPMAP_NEAREST);
            //this.gl.generateMipmap(this.gl.TEXTURE_2D);
            this.gl.bindTexture(this.gl.TEXTURE_2D, null);
            console.log("Updated texture (depth buffer)");
        });
        image.src = "data:image/png;base64," + depthBufferAsText;
        // console.log("Depth buffer set");
    },


     setRGBimage: function(imageAsText) {
         console.log("setRGBimage: Setting image, count = " + this._depthBufferCounter);
         var image = new Image();
         image.onload = dojo.hitch(this, function() {
             this.gl.bindTexture(this.gl.TEXTURE_2D, this.rgbTexture);
             this.gl.texImage2D(this.gl.TEXTURE_2D, 0, this.gl.RGBA, this.gl.RGBA, this.gl.UNSIGNED_BYTE, image);
             this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MAG_FILTER, this.gl.NEAREST);
             this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.NEAREST);
             //this.gl.generateMipmap(this.gl.TEXTURE_2D);
             this.gl.bindTexture(this.gl.TEXTURE_2D, null);
             console.log("Updated texture (image)");
         });
         image.src = "data:image/png;base64," + imageAsText;
         // console.log("Image set");
     },


    setViewMat: function( viewMatAsText, projMatAsText ) {
        console.log("proxyRenderer::setViewMat: Setting view matrix from server, count = " + this._depthBufferCounter);
        console.log("                 view mat: " + viewMatAsText);
        console.log("                 proj mat: " + projMatAsText);
        if (this._depth_matrices) {

            // Don't understand why these don't work...?!
            // console.log("     _depth_matrices.view: " + this._depth_matrices.m_from_world );
            // console.log("     _depth_matrices.proj: " + this._depth_matrices.m_projection.toString() );

            // Cumbersome... but works...
            var mv = " ";
            var pm = " ";
            var i;
            for (i=0; i<16; i++) {
                mv = mv + " " + this._depth_matrices.m_from_world[i]; // .toPrecision(5); // Why does this cause the code to "crash"?
                pm = pm + " " + this._depth_matrices.m_projection[i]; // .toPrecision(5);
            }
            console.log("     _depth_matrices.view:" + mv );
            console.log("     _depth_matrices.proj:" + pm );

            // Overriding the current values in _depth_matrices. These are not the current *client-side* values, but should have been
            // set to the server-side values from when the depth buffer was generated. This was done by subscription to "/model/updateSendStart"
            // above. Does that not produce the correct values?
            // (Hmm. Could the problem be that the subscription "callback" is not triggered until some period of user interaction inactivity
            // occurs, so that it does not get updated while the user manipulates the scene?)
            // Now overriding with values passed along together with that actual rgb+depth buffers.
            this._depth_matrices.m_projection = projMatAsText.split(/ /);
            this._depth_matrices.m_projection_inverse = mat4.inverse(mat4.create( this._depth_matrices.m_projection ));
            this._depth_matrices.m_from_world = viewMatAsText.split(/ /);
            this._depth_matrices.m_to_world = mat4.inverse(mat4.create( this._depth_matrices.m_from_world ));

        }
     },


    render: function(matrices) {
        this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);

        if ( (!this._splatProgram) && (this._splat_vs_src) && (this._splat_fs_src) ) {
            this.compileShaders();
        }

        if ( (this._depth_matrices) && (this._splatProgram) ) {
            this.gl.useProgram(this._splatProgram);
            var vertexPositionAttribute = this.gl.getAttribLocation( this._splatProgram, "aVertexPosition" );
            this.gl.enableVertexAttribArray( vertexPositionAttribute );

            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this._splatVertexBuffer);

            this.gl.activeTexture(this.gl.TEXTURE0);
            this.gl.bindTexture(this.gl.TEXTURE_2D, this.depthTexture);
            this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "uSampler"), 0 );

            this.gl.activeTexture(this.gl.TEXTURE1);
            this.gl.bindTexture(this.gl.TEXTURE_2D, this.rgbTexture);
            this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "rgbImage"), 1 );

            if (this.gl.getUniformLocation(this._splatProgram, "MV")) {
                this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "MV"), false, matrices.m_from_world );
            }
            if (this.gl.getUniformLocation(this._splatProgram, "PM")) {
                this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "PM"), false, matrices.m_projection );
            }
            if (this.gl.getUniformLocation(this._splatProgram, "depthPMinv")) {
                this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "depthPMinv"), false, this._depth_matrices.m_projection_inverse );
            }
            if (this.gl.getUniformLocation(this._splatProgram, "depthMVinv")) {
                this.gl.uniformMatrix4fv( this.gl.getUniformLocation(this._splatProgram, "depthMVinv"), false, this._depth_matrices.m_to_world );
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

            this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);

            // GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset
            this.gl.vertexAttribPointer( vertexPositionAttribute, 2, this.gl.FLOAT, false, 0, 0);
            // this.gl.enable(this.gl.POINT_SPRITE);
            // this.gl.enable(this.gl.VERTEX_PROGRAM_POINT_SIZE);
            this.gl.drawArrays(this.gl.POINTS, 0, this._splats_x*this._splats_y);
        }

        // console.log("rendering");
    }

});
