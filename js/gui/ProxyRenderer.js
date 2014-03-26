dojo.declare("gui.ProxyRenderer", null, {
    constructor: function(glContext, exposedModel, viewerKey) {
        this.gl = glContext;

        this.shaderFSSrc = "varying highp vec2 vTextureCoord;\n" +
                "uniform sampler2D uSampler;\n" +
                "void main(void) {\n" +
                "   gl_FragColor = texture2D(uSampler, vec2(vTextureCoord.s, 1.0-vTextureCoord.t));\n" +
                "   gl_FragColor.xy = gl_FragColor.yx;\n"+
                "}\n";

        this.shaderVSSrc = "attribute vec2 aVertexPosition;\n" +
                "varying highp vec2 vTextureCoord;\n" +
                "void main(void) {\n" +
                "   vec2 tmp = vec2(2.0 * aVertexPosition.x - 1.0, 2.0 * aVertexPosition.y - 1.0);\n" +
                "   gl_Position = vec4(tmp, 0.0, 1.0);\n" +
                "   vTextureCoord = aVertexPosition.xy;\n" +
                "}\n";



        dojo.subscribe("/model/updateSendStart", dojo.hitch(this, function(xml) {
            this.matrices = exposedModel.getElementValue(viewerKey);
            console.log("We are in subscriber");
        }));


        this.depthTexture = this.gl.createTexture();
//        depthImage = new Image();
//        depthImage.onload = function() {
//          handleTextureLoaded(cubeImage, cubeTexture);
//        }
//        depthImage.src = "cubetexture.png";



        this.vertexBuffer = this.gl.createBuffer();
        this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.vertexBuffer);

        this.textureCoordinates = [
            // Front
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

        // Send the source to the shader object

        this.gl.shaderSource(shaderFS, this.shaderFSSrc);
        // Compile the shader program

        this.gl.compileShader(shaderFS);
        if (!this.gl.getShaderParameter(shaderFS, this.gl.COMPILE_STATUS)) {
            alert("An error occurred compiling the shadersFS: " + this.gl.getShaderInfoLog(shaderFS));
            return null;
        }

        this.gl.shaderSource(shaderVS, this.shaderVSSrc);

        // Compile the shader program

        this.gl.compileShader(shaderVS);
        if (!this.gl.getShaderParameter(shaderVS, this.gl.COMPILE_STATUS)) {
            alert("An error occurred compiling the shadersVS: " + this.gl.getShaderInfoLog(shaderVS));
            return null;
        }

        // See if it compiled successfully

        this.shaderProgram = this.gl.createProgram();
        this.gl.attachShader(this.shaderProgram, shaderVS);
        this.gl.attachShader(this.shaderProgram, shaderFS);
        this.gl.linkProgram(this.shaderProgram);

        // If creating the shader program failed, alert

        if (!this.gl.getProgramParameter(this.shaderProgram, this.gl.LINK_STATUS)) {
            alert("Unable to initialize the shader program.");
        }

        this.gl.useProgram(this.shaderProgram);

        this.vertexPositionAttribute = this.gl.getAttribLocation(this.shaderProgram, "aVertexPosition");
        this.gl.enableVertexAttribArray(this.vertexPositionAttribute);

       
        console.log("Constructor ended");
    },
    setDepthBuffer: function(depthBufferAsText) {

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
        this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);

        this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.vertexBuffer);
       

        // Set the texture coordinates attribute for the vertices.

        //this.gl.bindBuffer(this.gl.ARRAY_BUFFER, cubeVerticesTextureCoordBuffer);
        //this.gl.vertexAttribPointer(this.textureCoordAttribute, 2, this.gl.FLOAT, false, 0, 0);

        // Specify the texture to map onto the faces.

        this.gl.activeTexture(this.gl.TEXTURE0);
        this.gl.bindTexture(this.gl.TEXTURE_2D, this.depthTexture);
        this.gl.uniform1i(this.gl.getUniformLocation(this.shaderProgram, "uSampler"), 0);

        // Draw the two-triangle "quad".

        this.gl.vertexAttribPointer(this.vertexPositionAttribute, 2, this.gl.FLOAT, false, 0, 0);
        this.gl.drawArrays(this.gl.TRIANGLES, 0, 6);

        console.log("rendering");
    }

});
