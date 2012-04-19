dojo.require("dijit.form.CheckBox");
dojo.require("dijit._Widget");

dojo.provide("gui.CheckBoxWithLabel");
dojo.declare("gui.CheckBoxWithLabel", [dijit._Widget], {
    constructor: function() {
        this._checkBox = new dijit.form.CheckBox();
    },
    
    buildRendering: function() {
        this.domNode = dojo.create("span");
        this._label = dojo.create("label");
        this.domNode.appendChild(this._checkBox.domNode);
        this.domNode.appendChild(this._label);
        dojo.attr(this._label, "for", this._checkBox.domNode.id);
    },
    
    
    on : function(event, func) {
        this._checkBox.on(event, func);
    },
    
    set : function(slot, value) {
        if(slot == "label") {
            this._setLabel(value);
        }
        else {
            this._checkBox.set(slot, value);
        }
    },
    
    get: function(slot) {
        if(slot=="label") {
            return this._label.innerHTML;
        }
        else {
            return this._checkBox.get(slot);
        }
    },
    
    _setLabel : function(value) {
        this._label.innerHTML = value;
    }
    
    
});