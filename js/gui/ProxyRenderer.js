dojo.declare("gui.ProxyRenderer", null, {


    constructor: function(glContext, exposedModel, viewerKey) {

        this._depthBufferCounter = 0;
        this._subscriptionCounter = 0;
        this._splats = 100;

        this.gl = glContext;

        var splat_vs_src =
                "attribute vec2 aVertexPosition;\n" +
                "varying highp vec2 vTextureCoord;\n" + // Implicitly taken to be *output*?!
                "uniform mat4 PM;\n" +
                "uniform mat4 MV;\n" +
                "uniform mat4 depthPMinv;\n" +
                "uniform mat4 depthMVinv;\n" +
                "uniform sampler2D uSampler;\n" +
                "varying highp float depth;\n" +
                "void main(void) {\n" +

                "    vec2 st = 0.5*(aVertexPosition.xy+1.0);\n" +
                "    st.y=1.0-st.y; \n" +
                "    vTextureCoord = st;\n" +
                "    gl_PointSize = 4.0;\n" +

//                "    // 8-bit version. Remember to fix in depth-reading code also, if changed.\n" +
//                "    depth = texture2D( uSampler, st ).r;\n" +

                "    // 16-bit version. Remember to fix in depth-reading code also, if changed.\n" +
                "    depth = ( texture2D( uSampler, st ).r +\n" +
                "              texture2D( uSampler, st ).g / 255.0 );\n" +

                "    // 24-bit version. Remember to fix in depth-reading code also, if changed.\n" +
//                "    depth = ( texture2D( uSampler, st ).r +\n" +
//                "              texture2D( uSampler, st ).g / 255.0 +\n" +
//                "              texture2D( uSampler, st ).b / (255.0*255.0) );\n" +

                "    // We may think of the depth texture as a grid of screen space points together with\n" +
                "    // depths, which we will subsample in order to get a sparser set of 'splats'.\n" +
                "    // First, we obtain ndc coordinates.\n" +
                "    float x_ndc = aVertexPosition.x;\n" +
                "    float y_ndc = aVertexPosition.y;\n" +
                "    float z_ndc = 2.0*depth - 1.0;\n" +

                "    // We have (x, y, z, 1)_{ndc, before}, and backtrace to world space 'before' for this point.\n" +
                "    vec4 vert_eb = depthPMinv * vec4( x_ndc, y_ndc, z_ndc, 1.0 );\n" +
                "    vec4 vert_wb = depthMVinv * vert_eb;\n" +

                "    // Next, we apply the current transformation to get the proxy splat.\n" +
                "    gl_Position = PM * MV * vert_wb;\n" +

                "}\n";

        var splat_fs_src =
                "uniform sampler2D rgbImage;\n" +
                "varying highp vec2 vTextureCoord;\n" +
                "varying highp float depth;\n" +
                "void main(void) {\n" +
                "    // gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);\n" + // blue

                // Hmm. Even when this is disabled, we still don't get all splats rendered. Why is this so? Shouldn't it be necessary with the discard here?!
                // Ah. The explanation is that the splats are really rendered, but with the background color, so they are not visible!
                "    if ( depth > 0.999 ) {\n" +
                "        // The depth should be 1 for fragments not rendered. It may be a problem that depth\n" +
                "        // input is 'varying'.\n" +
                "        discard;\n" +
                "        // gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n" + // white
                "        // return;\n" +
                "    }\n" +

                "    // Just to see if this ever happens...\n" +
//                "    if ( depth < 0.01 ) {\n" +
//                "        gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);\n" + // yellow
//                "        return;\n" +
//                "    }\n" +

                "    gl_FragColor = texture2D( rgbImage, vTextureCoord );\n" +
                "}\n";


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
        this._splatCoordinates = new Float32Array( this._splats*this._splats*2 );
        for (i=0; i<this._splats; i++) {
            for (j=0; j<this._splats; j++) {
                this._splatCoordinates[(this._splats*i+j)*2     ] = (2.0*j)/this._splats - 1.0;
                this._splatCoordinates[(this._splats*i+j)*2 + 1 ] = (2.0*i)/this._splats - 1.0;
            }
        }
        this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(this._splatCoordinates), this.gl.STATIC_DRAW);

        var splat_fs = this.gl.createShader(this.gl.FRAGMENT_SHADER);
        this.gl.shaderSource(splat_fs, splat_fs_src);
        this.gl.compileShader(splat_fs);
        if (!this.gl.getShaderParameter(splat_fs, this.gl.COMPILE_STATUS)) {
            alert("An error occurred compiling the splat_fs: " + this.gl.getShaderInfoLog(splat_fs));
            return null;
        }

        var splat_vs = this.gl.createShader(this.gl.VERTEX_SHADER);
        this.gl.shaderSource(splat_vs, splat_vs_src);
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

        console.log("Constructor ended");
    },


    setDepthBuffer: function(depthBufferAsText) {
        this._depthBufferCounter++;
        console.log("setDepthBuffer: Setting buffer, count = " + this._depthBufferCounter);
        var image = new Image();
        image.onload = dojo.hitch(this, function() {
            this.gl.bindTexture(this.gl.TEXTURE_2D, this.depthTexture);
            this.gl.texImage2D(this.gl.TEXTURE_2D, 0, this.gl.RGBA, this.gl.RGBA, this.gl.UNSIGNED_BYTE, image);
            this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MAG_FILTER, this.gl.LINEAR);
            this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.LINEAR_MIPMAP_NEAREST);
            this.gl.generateMipmap(this.gl.TEXTURE_2D);
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
             this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MAG_FILTER, this.gl.LINEAR);
             this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.LINEAR_MIPMAP_NEAREST);
             this.gl.generateMipmap(this.gl.TEXTURE_2D);
             this.gl.bindTexture(this.gl.TEXTURE_2D, null);
             console.log("Updated texture (image)");
         });
         image.src = "data:image/png;base64," + imageAsText;
         // console.log("Image set");
     },


    render: function(matrices) {
        this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);

        // ----------------------------------- Rendering splats -----------------------------------------

        if ( this._depth_matrices ) {
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

            this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);

            // GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset
            this.gl.vertexAttribPointer( vertexPositionAttribute, 2, this.gl.FLOAT, false, 0, 0);
            // this.gl.enable(this.gl.POINT_SPRITE);
            // this.gl.enable(this.gl.VERTEX_PROGRAM_POINT_SIZE);
            this.gl.drawArrays(this.gl.POINTS, 0, this._splats*this._splats);
        }

        // console.log("rendering");
    }

});
