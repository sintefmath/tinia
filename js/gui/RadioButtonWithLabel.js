
dojo.require("dijit.form.RadioButton");
dojo.require("dijit._Widget");

dojo.provide("gui.RadioButtonWithLabel");
dojo.declare("gui.RadioButtonWithLabel", [dijit._Widget], {
    constructor: function(options) {
        this._radioButton = new dijit.form.RadioButton(options);
        this._labelValue = options.label;
    },
    
    buildRendering: function() {
        this.domNode = dojo.create("span");
        this._label = dojo.create("label");
        this.domNode.appendChild(this._radioButton.domNode);
        this.domNode.appendChild(this._label);
        dojo.attr(this._label, "for", this._radioButton.domNode.id);
        this._setLabel(this._labelValue);
    },
    
    
    on : function(event, func) {
        this._radioButton.on(event, func);
    },
    
    set : function(slot, value) {
        if(slot == "label") {
            this._setLabel(value);
        }
        else {
            this._radioButton.set(slot, value);
        }
    },
    
    _setLabel : function(value) {
        this._labelValue = value;
        this._label.innerHTML = value;
    }
    
    
});