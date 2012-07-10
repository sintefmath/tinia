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

dojo.provide("gui.GUIBuilder");


dojo.require("dijit.layout.BorderContainer");
dojo.require("dijit.layout.ContentPane");
dojo.require("dijit.layout.TabContainer");
dojo.require("dijit.form.TextBox");
dojo.require("dojo.fx.Toggler");
dojo.require("dijit.form.DropDownButton");
dojo.require("dijit.TooltipDialog");
dojo.require("dijit.form.CheckBox");
dojo.require("dijit.form.ToggleButton");
dojo.require("dijit.form.RadioButton");
dojo.require("dijit.form.Select");
dojo.require("dijit.form.NumberSpinner");
dojo.require("dijit.form.HorizontalSlider");
dojo.require("dijit.form.VerticalSlider");
dojo.require("dijit.layout.TabContainer");
dojo.require("dijit.layout.TabController");
dojo.require("dijit.form.Button");


dojo.require("gui.CheckBoxWithLabel");
dojo.require("gui.VerticalLayout");
dojo.require("gui.HorizontalLayout");
dojo.require("gui.Label");
dojo.require("gui.GridLayout");
dojo.require("gui.RadioButtonWithLabel");
dojo.require("gui.Canvas");


dojo.declare("gui.GUIBuilder", null, {
    constructor: function(modelLib, renderListURL, localMode, urlHandler, renderList) {
        this._modelLib = modelLib;
        this._renderListURL = renderListURL;
        this._localMode = !!localMode;
        this._urlHandler = urlHandler;
	this._showRenderList = renderList;
    },
    
    buildGUI: function( root) {
        var element = null;
        if(!root) {
            console.log("ERROR");
            return null;
        }
        switch(root.type()) {
            case 'ElementGroup':
                element = this._makeElementGroup(root);
                break;
            case 'HorizontalLayout':
                element = this._makeHorizontalLayout(root);
                break;
            case 'VerticalLayout':
                element = this._makeVerticalLayout(root);
                break;
            case 'TextInput':
                element = this._addKeyValue(new dijit.form.TextBox(), root);
                break;
            case 'Grid':
                element = this._makeGridLayout(root);
                break;
            case 'PopupButton':
                element = this._makePopupButton(root);
                break;
            case 'Checkbox':
                element = this._addLabel(this._addKeyValue(new gui.CheckBoxWithLabel(), root, "checked", ["clicked", "change"]), 
                    root, 'label');
                break;
            case 'Label':
                element = this._addKeyValue(new gui.Label(), root, "value", "change");
                break;
            case 'ComboBox':
                element = this._makeComboBox(root);
                break;
            case 'RadioButtons':
                element = this._makeRadioButtons(root);
                break;
            case 'DoubleSpinBox':
            case 'SpinBox':
                element = this._makeSpinBox(root);
                break;
            case 'HorizontalSlider':
                element = this._makeHorizontalSlider(root);
                break;
            case 'VerticalSlider':
                element = this._makeVerticalSlider(root);
                break;
            case 'TabLayout':
                element = this._makeTabLayout(root);
                break;
            case 'Button':
                element = this._makeButton(root);
                break;
            case 'Canvas':
                element = new gui.Canvas({"key" : root.key(), 
                                          "renderlistKey" : root.renderlistKey(), 
                                           "boundingboxKey": root.boundingBoxKey(),
                                           "modelLib" : this._modelLib,
                                            "localMode" : this._localMode,
                                            "renderListURL" : this._renderListURL,
                                            "resetViewKey"  : root.resetViewKey(),
                                          "urlHandler" : this._urlHandler,
					  "showRenderList" : this._showRenderList,
                                            "scripts" : root.scripts()});
                
                break;
                
        }
        if(element) {
            this._addVisibilityToggles(element, root);
        }
        else {
            console.log("ERROR, type=" + root.type());
        }
        return element;
    },
    
    
    _makeElementGroup: function(root) {
        var heading = dojo.create("b");
        if(root.showValue()) {
            heading.innerHTML = this._modelLib.getValue(root.key());
            this._modelLib.addLocalListener(root.key(), function(key, value){
                heading.innerHTML = value;
            });
        }
        else {
            heading.innerHTML = this._modelLib.getAnnotation(root.key());
        }
        var container = new gui.VerticalLayout();
        dojo.style(heading, "padding-top", "5px");
        dojo.style(heading, "padding-bottom", "2px");
        dojo.style(heading, "padding-left", "0px");
        dojo.style(heading, "display", "block");
        container.domNode.appendChild(heading);
        var child = this.buildGUI(root.child());
        if(child) {
            container.addChild(child);
        }
        return container;
    },
    
    
    _makeButton : function(root) {
        var button = new dijit.form.Button();
        var label = "";
        if(root.showValue()) {
            this._modelLib.addLocalListener(root.key(), function(key, value) {
                button.set("label", value);
            });
            label = this._modelLib.getValue(root.key());
        }
        else {
            label = this._modelLib.getAnnotation(root.key());
        }
        button.set("label", label);
        button.on("click", dojo.hitch(this, function(event)  {
            this._modelLib.updateElement(root.key(), false);
            this._modelLib.updateElement(root.key(), true);
        }));
        return button;
    },
    
    
    _makeTabLayout : function(root) {
        var tabLayout = new dijit.layout.TabContainer({doLayout: false,
                                                        controllerWidget: "dijit.layout.TabController"
                                                       });
        
        for(var i = 0; i < root.length(); i++) {
            if(root.child(i)) {
                var element = null;
                if(root.child(i).child()) {
                    element = this.buildGUI(root.child(i).child());
                }

                var options = {
                    "title"      : this._modelLib.getAnnotation(root.child(i).key()),
                    "closeable" : false,
                    "selected"  : i==0
                    };
                options.content = element ? element.domNode : "";
                var contentPane = new dijit.layout.ContentPane(options);

                tabLayout.addChild(contentPane);
            }
            
        }
        var content = new gui.VerticalLayout();
        content.addChild(tabLayout);
        return content;
    },
    
    
    _makeSpinBox: function(root) {
        var spinBox = null;
        var style = "width: 7em;";
        if(this._modelLib.isConstrained(root.key())) {
            var max = this._modelLib.getMax(root.key());
            var min = this._modelLib.getMin(root.key());
            spinBox = new dijit.form.NumberSpinner({
                constraints : {
                    "max" : max,
                    "min" : min
                },
                "style" : style 
                
            });
        } else {
            spinBox = new dijit.form.NumberSpinner({"style" : style});
        }
        
        

        return this._addMaxMinListeners(this._addKeyValue(spinBox, root, "value", ["change", "click"]), root);
    },
    _makeHorizontalSlider: function(root) {

        var max = this._modelLib.getMax(root.key());
        var min = this._modelLib.getMin(root.key());
        var slider = new dijit.form.HorizontalSlider({
            "maximum" : max,
            "minimum" : min,
            "style"   : "width: 150px;"
        });
        return this._addMaxMinListeners(this._addKeyValue(slider, root, "value", "change" ), root);
    },
    
    _makeVerticalSlider: function(root) {

        var max = this._modelLib.getMax(root.key());
        var min = this._modelLib.getMin(root.key());
        var slider = new dijit.form.VerticalSlider({
            "maximum" : max,
            "minimum" : min,
            "style"   : "height: 150px;"
        });
        return this._addMaxMinListeners(this._addKeyValue(slider, root, "value", "change"), root);
    },
   
    _makeVerticalLayout: function(root) {
        var layout = new gui.VerticalLayout();

 
        for(var i = 0; i < root.length(); i++) {
            var widget = this.buildGUI(root.child(i));
            if(widget) {
                layout.addChild(widget);
            }
        }
        
        return layout;
    },
    
    _makeGridLayout : function(root) {
        var layout = new gui.GridLayout(root.height(), root.width());
        
        for(var i = 0; i < root.height(); i++) {
            for(var j = 0; j < root.width(); j++) {
                if(root.child(i,j)) {
                    var widget = this.buildGUI(root.child(i,j));
                    if(widget) {
                        layout.addChild(widget, i, j);
                    } else {
                        layout.addChild(new gui.EmptyWidget(), i, j);
                    }
                }
                else {
                    layout.addChild(new gui.EmptyWidget(), i, j);
                }
            }
        }
        return layout;
    },
    
    _makeHorizontalLayout : function(root) {
        var layout = new gui.HorizontalLayout();
        for(var i = 0; i < root.length(); i++) {
            var widget = this.buildGUI(root.child(i));
            if(widget) {
                layout.addChild(widget);
            }
        }
        return layout;
        
    },
    
    _makePopupButton: function(root) {
        var button = dijit.form.DropDownButton();
        button.set('label', root.showValue() ? this._modelLib.getValue(root.key()) :
            this._modelLib.getAnnotation(root.key()));
        
        if(root.showValue()) {
            this._modelLib.addLocalListener(root.key(), function(key, value) {
                button.set("label", value);
            });
        }
        
        var dialog = new dijit.TooltipDialog();
        button.set("dropDown", dialog);
        var child = this.buildGUI(root.child());
        if(child) {
            dialog.set('content', child);
        }
        return button;
    },
    
    _makeComboBox : function(root) {
        var restriction = this._modelLib.getRestriction(root.key());
        var options = [];
        for(var i = 0; i < restriction.length; i++) {
            options[i]=  {
                label: restriction[i], 
                value: restriction[i], 
                selected: false
            };
            if(this._modelLib.getValue(root.key())== options[i].value) {
                options.selected = true;
            }
        }
        var combobox = new dijit.form.Select({
            "name": root.key(), 
            "options" : options, 
            maxHeight:100, 
            selected: this._modelLib.getValue(root.key()), 
            value: this._modelLib.getValue(root.key())
        });
        return this._addKeyValue(combobox, root, "value", ["change", "blur"]);
    },
    
    _makeRadioButtons : function(root) {
        var layout = new gui.VerticalLayout();
        var restriction = this._modelLib.getRestriction(root.key());
        for(var i = 0; i < restriction.length; i++) {
            (dojo.hitch(this, function() { // For closure (need options and radiobutton)
                var options = {
                    name: "model."+root.key(), 
                    value: restriction[i],
                    label: restriction[i], 
                    checked : false
                };
                if(options.value == this._modelLib.getValue(root.key())) {
                    options.checked = true;
                }
                var radioButton = new gui.RadioButtonWithLabel(options);
                radioButton.on("change", dojo.hitch(this, function(value) {
                    if(value) {
                        this._modelLib.updateElement(root.key(), options.value);
                    }
                }));
                this._modelLib.addLocalListener(root.key(), function(key, value) {
                    if(value == options.value) {
                       
                        radioButton.set("checked", value);
                    }
                }, this);
                layout.addChild(radioButton);
            }))();
        }
        return layout;
    },
    
    
    
    
    _addKeyValue: function(widget, root, slot, event) {
        if(!slot) {
            slot = "value";
        }
        if(!event) {
            event = "change";
        }
        if(typeof event == "string") {
            event = [event];
        }
        if(root.showValue()) {
            
            for(var i = 0; i < event.length; i++) {
                var currentEvent = event[i];
                (dojo.hitch(this, function() {
                    widget.on(currentEvent, dojo.hitch(this, function(value) {
                        this._modelLib.updateElement(root.key(), widget.get(slot));
                    }));
                }))();
            }
            
            (dojo.hitch(this, function() {
                widget.on("keypress", dojo.hitch(this, function(event) {
                    if(event.keyCode == 13) {
                        this._modelLib.updateElement(root.key(), widget.get(slot));
                    }
                }));
            }))();

            
            
            this._modelLib.addLocalListener(root.key(), function(key, value) {
                widget.set(slot, value);
            }, this);
            
            widget.set(slot, this._modelLib.getValue(root.key()));
        } 
        else {
            widget.set(slot, this._modelLib.getAnnotation(root.key(), "en"));
        }
        return widget;
    },
    
    _addMaxMinListeners : function(widget, root) {
        this._modelLib.addLocalListener(root.key(), function(key, value) {
            var max = this._modelLib.getMax(key);
            var min = this._modelLib.getMin(key);
            widget.set("minimum", this._modelLib.getMin(key));
            widget.set("maximum", this._modelLib.getMax(key));
            widget.set("constraints", {"max" : max, "min": min} );
        }, this);
        return widget;
    },
    
    
    
    _addVisibilityToggles : function(widget, root) {
        if(root.visibilityKey() && root.visibilityKey().length > 0) {
            this._modelLib.addLocalListener(root.visibilityKey(), function(key, value) {
                if(root.visibilityKeyInverted() == value) {
                    dojo.style(widget.domNode, "display", "none");
                }
                else {
                    dojo.style(widget.domNode, "display", "block");
                }
            });
            if(this._modelLib.getValue(root.visibilityKey()) == root.visibilityKeyInverted()) {
                dojo.style(widget.domNode, "display", "none");
            }
        }
        if(root.enabledKey() && root.enabledKey().length > 0) {
            this._modelLib.addLocalListener(root.enabledKey(), function(key, value) {
                if(root.enabledKeyInverted() == value) {
                    widget.set("disabled", true);
                }
                else {
                    widget.set("disabled", false);
                }
            });
            if(this._modelLib.getValue(root.enabledKey()) == root.enabledKeyInverted()) {
                widget.set("disabled", true);
            }
            
        }
    },
    
    _addLabel: function(widget, root,  slot) {
        if(!slot) {
            slot = "label";
        }
        widget.set(slot, this._modelLib.getAnnotation(root.key()));
        return widget;
    }
});



dojo.declare("gui.EmptyWidget", [dijit._Widget], {});


