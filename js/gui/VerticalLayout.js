dojo.provide("gui.VerticalLayout");
dojo.require("dijit._Widget");
dojo.require("dijit._Container");

dojo.declare("gui.VerticalLayout", [dijit._Widget, dijit._Container],
{
    buildRendering: function() {
        this.domNode = dojo.create("div");
        this._nodes = {}
        this._children = [];
        dojo.style(this.domNode, "display", "block");
        dojo.style(this.domNode, "float", "none");
        dojo.style(this.domNode, "clear", "both");
        dojo.style(this.domNode, "margin-top", "0px");
        dojo.style(this.domNode, "padding-top", "0px");

        
    },
    
    addChild: function(node) {
        this._nodes[node.domNode.id] = dojo.create("div");
        this._nodes[node.domNode.id].appendChild(node.domNode);
        this._nodes[node.domNode.id].appendChild(dojo.create("br"));
        this.domNode.appendChild(this._nodes[node.domNode.id]);
        this._children[this._children.length] = node;
    },
    
    removeChild : function(node) {
        this.domNode.removeChild(this._nodes[node.domNode.id]);
    },
    
    startup: function() {
        for(var i = 0; i < this._children.length; i++) {
            this._children[i].startup();
        }
    }
});


