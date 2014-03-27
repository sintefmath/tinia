dojo.declare("gui.ProxyRenderer", null, {


    constructor: function(glContext, exposedModel, viewerKey) {

        this._depthBufferCounter = 0;
        this._depthBufferCounterLimit = 3;
        this._splats = 20;
        this._cntr = 0;

        this.gl = glContext;

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

        this.shaderFSSrc =
             "varying highp vec2 vTextureCoord;\n" +
             "uniform sampler2D uSampler;\n" +
             "void main(void) {\n" +
             "   gl_FragColor = texture2D(uSampler, vec2(vTextureCoord.s, 1.0-vTextureCoord.t));\n" +
             "   gl_FragColor.xy = gl_FragColor.yx;\n"+
             "}\n";

        var splat_vs_src =
                "attribute vec2 aVertexPosition;\n" +
                "varying highp vec2 vTextureCoord;\n" +
                "uniform mat4 PM;\n" +
                "uniform mat4 MV;\n" +
                "void main(void) {\n" +
                "   gl_Position = PM * MV * vec4(aVertexPosition, 0.0, 1.0);\n" +
                "   gl_PointSize = 3.0;\n" +
                "   vTextureCoord = aVertexPosition.xy;\n" +
                "}\n";

        var splat_fs_src =
                "varying highp vec2 vTextureCoord;\n" +
                "uniform sampler2D uSampler;\n" +
                "void main(void) {\n" +
                "   gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);\n" +
                "   // gl_FragColor = texture2D(uSampler, vec2(vTextureCoord.s, 1.0-vTextureCoord.t));\n" +
                "   // gl_FragColor.xy = gl_FragColor.yx;\n"+
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


        // ------------- Mockup-shader showing depth buffer as a green quad filling the viewport ------------------

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
        this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(this.textureCoordinates), this.gl.STATIC_DRAW);
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

        // ------------- Shader for testing splatting with preloaded vertex buffer object ------------------

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
            alert("Unable to initialize the shader program.");
        }

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

        // ------------- Rendering two-triangle "quad" to cover viewport with colour-coded depth buffer -----------

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

        this.gl.vertexAttribPointer(this.vertexPositionAttribute, 2, this.gl.FLOAT, false, 0, 0);
        this.gl.drawArrays(this.gl.TRIANGLES, 0, 6);

        // ----------------------------------- Rendering splats -----------------------------------------

        this.gl.useProgram(this._splatProgram);
        var vertexPositionAttribute = this.gl.getAttribLocation( this._splatProgram, "aVertexPosition" );
        this.gl.enableVertexAttribArray( vertexPositionAttribute );

        this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this._splatVertexBuffer);
        this.gl.activeTexture(this.gl.TEXTURE0);
        this.gl.bindTexture(this.gl.TEXTURE_2D, this.depthTexture);
        this.gl.uniform1i( this.gl.getUniformLocation(this._splatProgram, "uSampler"), 0 ); // this.gl.TEXTURE0);

        loc = this.gl.getUniformLocation(this._splatProgram, "MV");
        if (loc) {
            // this.gl.uniformMatrix4fv( loc, false, /* transposition */ worldChange );
            this.gl.uniformMatrix4fv( loc, false, /* transposition */ matrices.m_from_world );
        }

        loc = this.gl.getUniformLocation(this._splatProgram, "PM");
        if (loc) {
            // this.gl.uniformMatrix4fv( loc, false, /* transposition */ worldChange );
            this.gl.uniformMatrix4fv( loc, false, /* transposition */ matrices.m_projection );
        }

        this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);

        // GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset
        this.gl.vertexAttribPointer( vertexPositionAttribute, 2, this.gl.FLOAT, false, 0, 0);
        this.gl.enable(this.gl.POINT_SPRITE);
        this.gl.enable(this.gl.VERTEX_PROGRAM_POINT_SIZE);
        this.gl.drawArrays(this.gl.POINTS, 0, this._splats*this._splats);


        // console.log("rendering");
    }

});
