dojo.declare("gui.ProxyRenderer", null, {
	constructor: function(glContext, exposedModel, viewerKey) {
		this.gl = glContext;
		
		
		dojo.subscribe("/model/updateSendStart", dojo.hitch(this, function(xml) {
			this.matrices = exposedModel.getElementValue(viewerKey);
			console.log("We are in subscriber");
		}));


        depthTexture = this.gl.createTexture();
//        depthImage = new Image();
//        depthImage.onload = function() {
//          handleTextureLoaded(cubeImage, cubeTexture);
//        }
//        depthImage.src = "cubetexture.png";


		console.log("Constructor ended");
	},
	

	setDepthBuffer: function(depthBufferAsText) {
		
		// Update texture

        this.gl.bindTexture(this.gl.TEXTURE_2D, this.depthTexture);
        this.gl.texImage2D(this.gl.TEXTURE_2D, 0, this.gl.RGBA, this.gl.RGBA, this.gl.UNSIGNED_BYTE, image);
        this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MAG_FILTER, this.gl.LINEAR);
        this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.LINEAR_MIPMAP_NEAREST);
        this.gl.generateMipmap(this.gl.TEXTURE_2D);

        rawDepth = textToRaw( depthBufferAsText );

        this.gl.bindTexture(this.gl.TEXTURE_2D, rawDepth);

		console.log("Depth buffer set");
		
	},
	

    render: function(matrices) {


		console.log("rendering");
	}

});
