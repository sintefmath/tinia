function TextureDrawer( params ) {
    console.log("Constructing Texturedrawer");

    this._model = params.exposedModel;
    //this._key = params.key;
    //var viewer = this._model.getElementValue(this._key);
}

TextureDrawer.prototype = {
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