/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    
    child: function(i) {
        return this._children[i];
    },
    
    startup: function() {
        for(var i = 0; i < this._children.length; i++) {
            this._children[i].startup();
        }
    }
})


