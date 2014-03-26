dojo.declare("gui.ProxyRenderer", null, {


    constructor: function(glContext, exposedModel, viewerKey) {

        this._depthBufferCounter = 0;
        this._depthBufferCounterLimit = 3;
        this._splats = 20;
        this._cntr = 0;

        this.gl = glContext;

        this.shaderFSSrc =
             "varying highp vec2 vTextureCoord;\n" +
             "uniform sampler2D uSampler;\n" +
             "void main(void) {\n" +
             "   gl_FragColor = texture2D(uSampler, vec2(vTextureCoord.s, 1.0-vTextureCoord.t));\n" +
             "   gl_FragColor.xy = gl_FragColor.yx;\n"+
             "}\n";
        this.shaderVSSrc =
             "attribute vec2 aVertexPosition;\n" +
             "varying highp vec2 vTextureCoord;\n" +
             "uniform mat4 MV;\n" +
             "void main(void) {\n" +
             "   vec2 tmp = vec2(2.0 * aVertexPosition.x - 1.0, 2.0 * aVertexPosition.y - 1.0);\n" +

             "   // gl_Position = MV * vec4(tmp, 0.0, 1.0);\n" +
             "   gl_Position = vec4(tmp, 0.0, 1.0);\n" +

             "   vTextureCoord = aVertexPosition.xy;\n" +
             "}\n";

        var splat_vs_src =
                "\n" +
                "in layout (location=2) vec3 position;\n" +
                "in layout (location=3) vec2 input_texcoo;\n" +
                "\n" +
                "out vec2 tex_coo;\n" +
                "out VF\n" +
                "{\n" +
                "    smooth float render;\n" +
                "} vert_out;\n" +
                "\n" +
                "uniform mat4 MV;\n" +
                "uniform mat4 PM;\n" +
                "\n" +
                "uniform sampler2D rgb_sampler;\n" +
                "uniform sampler2D depth_sampler;\n" +
                "uniform sampler2D q_hat_sampler;\n" +
                "\n" +
                "uniform int pt_size;\n" +
                "\n" +
                "void main()\n" +
                "{\n" +
                "\n" +
                "    vec3 new_pos = position;\n" +
                "    float depth  = texture2D( depth_sampler, input_texcoo ).x;\n" +
                "    vec3 q_hat   = texture2D( q_hat_sampler, input_texcoo ).xyz;\n" +
                "\n" +
                "    new_pos = q_hat;\n" +
                "\n" +
                "    vert_out.render = 1.0;\n" +
                "    gl_Position = PM * MV * vec4( new_pos, 1.0 );\n" +
                "    tex_coo = input_texcoo;\n" +
                "    gl_PointSize = pt_size;\n" +
                "\n" +
                "}\n";
        var splat_fs_src =
                "in vec2 tex_coo;\n" +
                "\n" +
                "in VF\n" +
                "{\n" +
                "    smooth float render;\n" +
                "} frag_in;\n" +
                "\n" +
                "out vec4 color;\n" +
                "\n" +
                "uniform sampler2D rgb_sampler;\n" +
                "uniform sampler2D q_hat_sampler;\n" +
                "uniform sampler2D depth_sampler;\n" +
                "\n" +
                "uniform int q_ring_cursor;\n" +
                "uniform float splat_marker_weight;\n" +
                "\n" +
                "void main()\n" +
                "{\n" +
                "    color = vec4(1.0);\n" +
                "\n" +
                "    if (frag_in.render<0.999)\n" +
                "        discard;\n" +
                "\n" +
                "    vec2 c = gl_PointCoord-vec2(0.5);   // c in [-0.5, 0.5]^2\n" +
                "    float r_squared = dot(c, c);        // r_squared in [0, 0.5], radius squared for the largest inscribed circle is 0.25\n" +
                "    if (r_squared>0.25)\n" +
                "        discard;\n" +
                "\n" +
                "    color.xyz = texture2D( rgb_sampler, tex_coo ).rgb; //  + vec3(0.1);\n" +
                "\n" +
                "    float attenuation = 1.0 - 4.0*r_squared;\n" +
                "\n" +
                "    attenuation = attenuation + 0.4;\n" +
                "\n" +
                "    \n" +
                "    // attenuation = 1.0; // Hiding the actual points\n" +
                "\n" +
                "    color.xyz = color.xyz * attenuation;\n" +
                "}\n";

        dojo.subscribe("/model/updateSendStart", dojo.hitch(this, function(xml) {
            this._depthBufferCounter++;
            console.log("Subscriber: _depthBufferCounter: " + this._depthBufferCounter);
            if ( this._depthBufferCounter > this._depthBufferCounterLimit ) {
                console.log("Subscriber: not setting matrices.")
                return;
            }
            console.log("Subscriber: setting matrices.")
            // this._matrices = exposedModel.getElementValue(viewerKey);
            var viewer = exposedModel.getElementValue(viewerKey);
            this._depth_matrices = {
                m_projection:         viewer.getElementValue("projection"),
                m_projection_inverse: mat4.inverse(mat4.create(viewer.getElementValue("projection"))),
                m_to_world:           mat4.inverse(mat4.create(viewer.getElementValue("modelview"))),
                m_from_world:         viewer.getElementValue("modelview")
            }
        }));


        this.depthTexture = this.gl.createTexture();


        this.vertexBuffer = this.gl.createBuffer();
        this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.vertexBuffer);
        this.textureCoordinates = [
            0.0, 0.0,
            1.0, 0.0,
            1.0, 1.0,
            0.0, 0.0,
            1.0, 1.0,
            0.0, 1.0
        ];
        this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(this.textureCoordinates),
                this.gl.STATIC_DRAW);
        var shaderFS = this.gl.createShader(this.gl.FRAGMENT_SHADER);
        var shaderVS = this.gl.createShader(this.gl.VERTEX_SHADER);
        this.gl.shaderSource(shaderFS, this.shaderFSSrc);
        this.gl.compileShader(shaderFS);
        if (!this.gl.getShaderParameter(shaderFS, this.gl.COMPILE_STATUS)) {
            alert("An error occurred compiling the shadersFS: " + this.gl.getShaderInfoLog(shaderFS));
            return null;
        }
        this.gl.shaderSource(shaderVS, this.shaderVSSrc);
        this.gl.compileShader(shaderVS);
        if (!this.gl.getShaderParameter(shaderVS, this.gl.COMPILE_STATUS)) {
            alert("An error occurred compiling the shadersVS: " + this.gl.getShaderInfoLog(shaderVS));
            return null;
        }
        this.shaderProgram = this.gl.createProgram();
        this.gl.attachShader(this.shaderProgram, shaderVS);
        this.gl.attachShader(this.shaderProgram, shaderFS);
        this.gl.linkProgram(this.shaderProgram);
        if (!this.gl.getProgramParameter(this.shaderProgram, this.gl.LINK_STATUS)) {
            alert("Unable to initialize the shader program.");
        }


        this.gl.useProgram(this.shaderProgram);
        this.vertexPositionAttribute = this.gl.getAttribLocation(this.shaderProgram, "aVertexPosition");
        this.gl.enableVertexAttribArray(this.vertexPositionAttribute);




        this.splatVertexBuffer = this.gl.createBuffer();
        this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.splatVertexBuffer);
        this._splatCoordinates = new Float32Array( this._splats*this._splats*3);

        for (i=0; i<this._splats; i++) {
            for (j=0; j<this._splats; j++) {
                this._splatCoordinates[(this._splats*i+j)     ] = 2.0*j/this._splats - 1.0;
                this._splatCoordinates[(this._splats*i+j) + 1 ] = 2.0*i/this._splats - 1.0;
                this._splatCoordinates[(this._splats*i+j) + 2 ] = 0.0;
            }
        }

//        this.splatCoordinates = [
//            0.0, 0.0,
//            1.0, 0.0,
//            1.0, 1.0,
//            0.0, 0.0,
//            1.0, 1.0,
//            0.0, 1.0
//        ];
        this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(this.splatCoordinates), this.gl.STATIC_DRAW);
//        var splat_fs = this.gl.createShader(this.gl.FRAGMENT_SHADER);
//        var splat_vs = this.gl.createShader(this.gl.VERTEX_SHADER);
//        this.gl.shaderSource(splat_fs, splat_fs_src);
//        this.gl.compileShader(splat_fs);
//        if (!this.gl.getShaderParameter(splat_fs, this.gl.COMPILE_STATUS)) {
//            alert("An error occurred compiling the splat_fs: " + this.gl.getShaderInfoLog(splat_fs));
//            return null;
//        }
//        this.gl.shaderSource(splat_vs, splat_vs_src);
//        this.gl.compileShader(splat_vs);
//        if (!this.gl.getShaderParameter(splat_vs, this.gl.COMPILE_STATUS)) {
//            alert("An error occurred compiling the splat_vs: " + this.gl.getShaderInfoLog(splat_vs));
//            return null;
//        }
//        this.splatProgram = this.gl.createProgram();
//        this.gl.attachShader(this.splatProgram, splat_vs);
//        this.gl.attachShader(this.splatProgram, splat_fs);
//        this.gl.linkProgram(this.splatProgram);
//        if (!this.gl.getProgramParameter(this.splatProgram, this.gl.LINK_STATUS)) {
//            alert("Unable to initialize the shader program.");
//        }



        console.log("Constructor ended");
    },


    setDepthBuffer: function(depthBufferAsText) {
        //this._depthBufferCounter++;
        if ( this._depthBufferCounter > this._depthBufferCounterLimit ) {
            console.log("setDepthBuffer: not setting depth buffer, counter too high");
            return;
        }
        console.log("setDepthBuffer: setting buffer, count = " + this._depthBufferCounter);
        var image = new Image();

        image.onload = dojo.hitch(this, function() {
            this.gl.bindTexture(this.gl.TEXTURE_2D, this.depthTexture);
            this.gl.texImage2D(this.gl.TEXTURE_2D, 0, this.gl.RGBA, this.gl.RGBA, this.gl.UNSIGNED_BYTE, image);
            this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MAG_FILTER, this.gl.LINEAR);
            this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.LINEAR_MIPMAP_NEAREST);
            this.gl.generateMipmap(this.gl.TEXTURE_2D);
            this.gl.bindTexture(this.gl.TEXTURE_2D, null);
            console.log("Updated texture");
        });
        image.src = "data:image/png;base64," + depthBufferAsText;
        console.log("Depth buffer set");
    },


    render: function(matrices) {
        this._cntr++;
        this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);

        this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.vertexBuffer);
        this.gl.activeTexture(this.gl.TEXTURE0);
        this.gl.bindTexture(this.gl.TEXTURE_2D, this.depthTexture);
        this.gl.uniform1i( this.gl.getUniformLocation(this.shaderProgram, "uSampler"), 0 ); // this.gl.TEXTURE0);

        var worldChange = mat4.create( matrices.m_from_world );
        if (this._depth_matrices) {
            mat4.multiply( worldChange, this._depth_matrices.m_to_world, worldChange );
        }

        var loc = this.gl.getUniformLocation(this.shaderProgram, "MV");
        if (loc) {
            this.gl.uniformMatrix4fv( loc, false, /* transposition */ worldChange );
        }

        // Draw the two-triangle "quad".
        this.gl.vertexAttribPointer(this.vertexPositionAttribute, 2, this.gl.FLOAT, false, 0, 0);
        this.gl.drawArrays(this.gl.TRIANGLES, 0, 6);

        // console.log("rendering");
    }

});
