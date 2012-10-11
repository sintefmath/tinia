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