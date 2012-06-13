dojo.provide("gui.HorizontalLayout");
dojo.require("dijit._Widget");
dojo.require("dijit._Container");

dojo.declare("gui.HorizontalLayout", [dijit._Widget, dijit._Container],
{
    buildRendering: function() {
        this.domNode = dojo.create("div");
        this._table = dojo.create("table");
        this._row = dojo.create("tr");
        this._table.appendChild(this._row);;
        this.domNode.appendChild(this._table);
        
        dojo.style(this.domNode, "display", "block");
        dojo.style(this.domNode, "float", "none");
        dojo.style(this.domNode, "clear", "both");

        this._nodes = {}
        this._lastNode = null;
        this._children = [];
    },
    
    addChild: function(node) {
        
        this._nodes[node.domNode.id] = dojo.create("td");
        dojo.style(this._nodes[node.domNode.id], "vertical-align", "top");
        this._nodes[node.domNode.id].appendChild(node.domNode);
        this._row.appendChild(this._nodes[node.domNode.id]);

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
    
});/* 
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */


