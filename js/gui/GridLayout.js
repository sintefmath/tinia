dojo.require("dijit._Widget");
dojo.require("dijit._Container");
dojo.provide("gui.GridLayout");


dojo.declare("gui.GridLayout", [dijit._Widget], {
    constructor: function(height, width) {
        this._GridLayout_height = height;
        this._GridLayout_width = width;
        this._children = [];
    },
    
    buildRendering: function() {
        this.params = {};
        this._GridLayout_children = [];
        this._GridLayout_cells = [];
        this.domNode = dojo.create("table");
        for(var i = 0; i < this._GridLayout_height; i++) {
            this._GridLayout_cells[i] = []
            var row = dojo.create("tr");
            for(var j = 0; j < this._GridLayout_width; j++) {
                var cell = dojo.create("td");
                this._GridLayout_cells[i][j] = cell;
                row.appendChild(cell);
            }
            this.domNode.appendChild(row);
       }
       
       

    },
    
    addChild : function(node, row, col) {

        this._GridLayout_children[this._GridLayout_children.length] = node;
        this._GridLayout_cells[row][col].appendChild(node.domNode);
        this._children[this._children.length] = node;
    },
    
    startup: function() {
        for(var i = 0; i < this._children.length; i++) {
            this._children[i].startup();
        }
    }
})


