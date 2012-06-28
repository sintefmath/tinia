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


