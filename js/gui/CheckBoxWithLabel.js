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