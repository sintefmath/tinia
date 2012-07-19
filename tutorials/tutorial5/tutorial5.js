function MouseClickResponder( params ) {
    this._model = params.exposedModel;    
}

MouseClickResponder.prototype = {
    mousePressEvent: function (event) {        
        this.updateModel(event.relativeX, event.relativeY);

    },

    mouseMoveEvent: function (event) {        
        this.updateModel(event.relativeX, event.relativeY);
    },

    updateModel: function (relX, relY) {
        this._model.updateElement("click_xy", relX + " " + relY);        
    }
}