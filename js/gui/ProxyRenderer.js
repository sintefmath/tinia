
dojo.declare("gui.ProxyRenderer", null, {
	constructor: function(glContext, exposedModel, viewerKey) {
		this.gl = glContext;
		
		
		dojo.subscribe("/model/updateSendStart", dojo.hitch(this, function(xml) {
			this.matrices = exposedModel.getElementValue(viewerKey);
			console.log("We are in subscriber");
		}));
		
		console.log("Constructor ended");
	},
	
	setDepthBuffer: function(depthBufferAsText) {
		
		// Update texture
		console.log("Depth buffer set");
		
	},
	
	render : function(matrices) {
		console.log("rendering");
	}

});