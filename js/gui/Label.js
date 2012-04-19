dojo.require("dijit._Widget");
dojo.provide("gui.Label");

dojo.declare("gui.Label", [dijit._Widget], {
    constructor: function() {
        
    },
    
    buildRendering: function() {
        this.domNode = dojo.create("label");
        
    },
    
    _setValueAttr: function(value) {
        this.domNode.innerHTML = value;
    },
    
    _getValueAttr : function() {
        return this.domNode.innerHTML;
    }
    
})

