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

dojo.provide("model.ExposedModel");
/**
 * This is the client side equivalent of the serverside model.
 */
dojo.declare("model.ExposedModel", null, {
    constructor: function() {
        this._legalTypes    = ["bool", "integer", "string", "composite", "float", "double"];
        this._listeners      = [];
        this._localListeners = {};
        this._elements = {};
        this._gui = null;
        this._revision = 0;
    },
    
    setRevision : function(revision) {
        this._revision = revision;
        for(var key in this._elements) {
            this._elements[key].setRevision(revision);
        }
    },
    
    getLocalRevision: function(key) {
        this._getElementData(key).getRevision();
    },
    
    getRevision: function() {
        return this._revision;
    },
    
    setGUI: function(gui) {
        this._gui = gui;
    },
    
    GUI : function() {
        return this._gui;
    },
    
    /**
     * @param type String
     * @param key String
     * @param initialValue type
     */
    addElement: function(type, key, initialValue) {
        if(!this._legalTypes.indexOf(type) < 0) {
            throw "Type " + type + " is not a recognized type.";
        }
        if(this._elements[key] !== undefined) {
            throw "Trying to add element with key " + key + " but "
            + "is already in the model.";
        }
        this._elements[key] = new model.ElementData(type, initialValue);
    },
    
    addElementWithRestriction: function(type, key, initialValue, restrictions) {
        this.addElement(type, key, initialValue);
        var elementData = this._getElementData(key);
        elementData.setRestriction(restrictions);
    },
    
    
    addConstrainedElement: function(type, key, initialValue, min , max) {
        this.addElement(type, key, initialValue);
        var elementData = this._getElementData(key);
        elementData.setMax(max);
        elementData.setMin(min);
    },
    
    updateRestrictions: function(key, restrictions) {
        var elementData = this._getElementData(key);
        var oldRestrictions = elementData.getRestriction();
        var changed = false;
        
        for(var i = 0; i < oldRestrictions.length; i++) {
            var found = false;
            for(var j = 0; j < restrictions.length; j++) {
                if(oldRestrictions[i] == restrictions[j]) {
                    found = true;
                }
            }
            if(!found) {
                changed = true;
            }
        }
        if(oldRestrictions.length != restrictions.length) {
            changed = true;
        }

        if(!changed) {
            return;
        }

        elementData.setRestriction(restrictions);
        
        this._notifyListeners(key, elementData);
    },
    
    updateConstrainedElement: function(key, min, max) {
        var elementData = this._getElementData(key);
        var emitChange = false;
        if(elementData.getMax() != max) {
            elementData.setMax(max);
            emitChange = true;
        } else if (elementData.getMin() != min) {
            elementData.setMin(min);
            emitChange = true;
        }
        
        if(emitChange) {
            this._notifyListeners(key, elementData);
        }
        
    },
    addListElement: function(type, key, length, list) {
        if(!list) {
            list = [];
        }
        if(this._elements[key]) {
            throw "Trying to add "  + key + " but it's already in the model";
        }
        if(this._legalTypes.indexOf(type) < 0) {
            throw "Type: " + type + " is not a legal type.";
        }
        this._elements[key] = new model.ElementData("list<"+type+">", list, length);
    },
    
    hasRestriction: function(key) {
        
        return this._getElementData(key).hasRestriction();
    },
    
    length: function(key) {
        return this._getElementData(key).length();
    },
    
    getRestriction: function(key) {
        return this._getElementData(key).getRestriction();
    },
    
    getValue: function(key) {
        return this._get(key);
    },
    
    getElementValue: function(key) {
        return this.getValue(key);
    },
    
    getType: function(key) {
        return this._getElementData(key).type();
    },
    
    updateElement: function(key, newValue) {
        
        var elementData = this._getElementData(key);
        var oldValue = this._get(key);
        elementData.setValue(newValue);
        var newValueStored = this._get(key);
        if(oldValue != newValueStored || elementData.type() == "composite") {
            this._notifyListeners(key, elementData);
        }
    },
    
    isList: function(key) {
        return this.getType(key).substring(0,4) == "list";
    },
    
    addAnnotation: function(key, annotation) {
        var elementData = this._getElementData(key);
        if(typeof(annotation) == "string") {
            elementData.setAnnotation("en", annotation);
        }
        else {
            // Assume map
            for(var language in annotation) {
                elementData.setAnnotation(language, annotation[language]);
            }
        }
    },
    

    getAnnotation: function(key, language) {
        var requestedLanguage = language ? language : "en";
        var annotation = key;
        var elementData = this._getElementData(key);
        try {
            annotation = elementData.annotation(requestedLanguage);
        } catch(e) {
            annotation = key;
        }
        return annotation;
    },
    
    keys: function() {
        var keys = [];
        for(var key in this._elements) {
            keys[keys.length] = key;
        }
        return keys;
    },
    
    isConstrained: function(key) {
        return this._getElementData(key).hasMaxMin();
    },
    
    
    getMax: function(key) {
        var elementData = this._getElementData(key);
        return this._cast(elementData.getMax(), elementData.type());
    },
    
    getMin: function(key) {
        var elementData = this._getElementData(key);
        return this._cast(elementData.getMin(), elementData.type());
    },
    
    hasKey: function(key) {
        return this._elements[key] !== undefined;
    },
    
    addListener: function(func, context) {
        if(context === undefined) {
            context = null;
        }
        this._listeners[this._listeners.length] = dojo.hitch(context, func);
    },
    
    addLocalListener: function(key, func, context) {
        if(!context) {
            context = null;
        }
        if(!this._localListeners[key]) {
            this._localListeners[key] = [];
        }
        this._localListeners[key][this._localListeners[key].length] = dojo.hitch(context, func);
    },
    
    
    // Private
    _getElementData: function(key) {
        var elementData = this._elements[key];
        if(elementData === undefined) {
            throw "Trying to access element " + key + " but it is not"
            + " in the model.";
        }
        return elementData;
    },
   
    _notifyListeners: function(key, elementData) {
        for(var i = 0; i < this._listeners.length; i++) {
            this._listeners[i](key, this._get(key));
        }
        if(this._localListeners[key]) {
            for(var i = 0; i <  this._localListeners[key].length; i++) {
                this._localListeners[key][i](key, this._get(key));
            }
        }
    },
   

    _get     : function(key)
    {
        var elementData = this._getElementData(key);
        return this._cast(elementData.value(), elementData.type());
        
    },
    
    _cast: function(value, type) {
        if(type.substring(0, 4) == "list") {
            type = type.substring(5);
            type = type.substring(0, type.length-1);
            for(var i = 0; i < value.length; i++) {
                value[i] = this._cast(value[i], type);
            }
            return value;
        }

        switch(type) {
            case 'bool':
                return ((value - 0) != 0);
            case 'string':
                return value;
            case 'integer':
                return Math.floor(value - 0);
            case 'double':
            case 'float':
                return value - 0.0;
            case 'composite':
                return value;
        }
    }
});

dojo.declare("model.ElementData", null, {

    constructor: function(type, value, length) {
        this._type = type;
        this._value = value;
        
        this._annotations = {};
        this._restriction = [];
        this._hasMaxMin   = false;
        if(length === undefined) {
            length = 1;
        }
        this._length = length;
        this._revision = 0;
    },
    
    value: function() {

        return this._value;
    },
    
    setValue: function(value) {
        this._revision++;
        this._value = value;
    },
    
    getRevision: function() {
        return this._revision;
    },
    
    setRevision: function(revision) {
        this._revision = revision;
    },
    
    type: function() {
        return this._type;
    },
    
    setAnnotation: function(language, annotation) {
        this._annotations[language] = annotation;
    },
    annotation: function(language) {
        var annot = this._annotations[language];
        if(!annot) {
            throw "Trying to access annotation for language " + 
            language + " but no such annotation exists.";
        }
        return annot;
    },
    
    setRestriction: function(restriction) {
        this._restriction = [];
        for(var i = 0; i < restriction.length; i++) {
            this._restriction[i] = restriction[i];
        }
    },
    
    hasRestriction: function() {
        return this._restriction.length > 0;
    },
    
    getRestriction: function() {
        return this._restriction;
    },
    
    setMax: function(max) {
        this._hasMaxMin = true;
        this._max = max;
    },
    
    setMin: function(min) {
        this._hasMaxMin = true;
        this._min = min;
    },
    
    getMax: function() {
        if(!this._hasMaxMin) {
            throw "Trying to access max, but this element does not have an upper bound";
        }
        return this._max;
    },
    
       
    getMin: function() {
        if(!this._hasMaxMin) {
            throw "Trying to access min, but this element does not have a lower bound";
        }
        return this._min;
    },
    
    hasMaxMin: function() {
        return this._hasMaxMin;
    },
    
    length: function() {
        return this._length;
    }
    
    
    
// Private

});

dojo.declare("model.Composite", model.ExposedModel, {});
