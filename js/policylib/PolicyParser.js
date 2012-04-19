/* 
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
dojo.require("policylib.PolicyLib");
dojo.require("policylib.gui.GUILayout");
dojo.provide("policylib.PolicyParser");
dojo.provide("policylib.StateParser");
dojo.provide("policylib.StateSchemaParser");
dojo.provide("policylib.GUIParser");


dojo.declare("policylib.XMLHelper", null, {
    queryXSD: function(xml, elementName) {
        var result = dojo.query(elementName, xml);
        if(!result) {
            result = dojo.query(elementName, xml);
        }
        return result;
    } 
});


dojo.declare("policylib.PolicyParser", policylib.XMLHelper, {
    constructor: function(policyLib) {
        this._stateSchemaParser = new policylib.StateSchemaParser(policyLib);
        this._stateParser = new policylib.StateParser(policyLib);
        this._guiParser = new policylib.GUIParser(policyLib);
        this._policyLib = policyLib;
    },
    
    parseXML: function(xml) {
        this._stateSchemaParser.parseXML(xml);
        this._stateParser.parseXML(xml);
        this._guiParser.parseXML(xml);
        this._addRevision(xml);
    },
    
    _addRevision: function(xml) {
        
        var policyUpdate = this.queryXSD(xml, "PolicyUpdate");
        if(!policyUpdate || policyUpdate.length == 0) {
            return;
        }
        var revision = dojo.attr(policyUpdate[0], "revision");
        if(!revision) {
            revision = 0;
        }
        
        revision = revision-0;
        this._policyLib.setRevision(revision);
    },
    
    // Private
    _stateSchemaParser  : null,
    _stateParser        : null,
    _guiParser          : null
});



dojo.declare("policylib.StateSchemaParser", policylib.XMLHelper, {
    constructor: function(policyLib) {
        this._policyLib = policyLib;
    },
    
    parseXML: function(xml) {

        var schema = this.queryXSD(xml, "schema");
        if(!schema || schema.length ==0) {
            return;
        }
        
        var elements = this.queryXSD(schema[0], "all")[0].childNodes;

        for(var i = 0; i < elements.length; i++) {
            
            this.addElement(this._policyLib, elements[i]);
        }
        
    },
    
    addElement: function(parent, xmlElement) {

        var nodeName = xmlElement.nodeName;
        if(!nodeName) {
            return;
        }
        
        nodeName = nodeName.replace("xsd:", "");
        if(this._elements.indexOf(nodeName) < 0) {
            return;
        }
        
        
        var key = dojo.attr(xmlElement, "name");

        if(!key) {
            console.log(xmlElement);
            throw "Missing key in xmlElement = " + xmlElement;
        }
        else if(parent.hasKey(key)) {
            
            // Nothing to do here
            return;
        }
        if(nodeName == "element") {
            if(this._isRestricted(xmlElement)) {
                this.addElementWithRestriction(parent, key, xmlElement);
            }
            else {
                
                var type = dojo.attr(xmlElement, "type");
            
                if(type && type.length > 0) {
                    type = type.replace("xsd:", "");

                    this.addSimpleElement(parent, type, key, xmlElement);
                }
            }
        }
        else {

            this.addComplexElement(parent, key, xmlElement);
        }
        
        this.addAnnotation(parent, key, xmlElement);
        
    },
    
    addSimpleElement: function(parent, type, key, xmlElement) {
        parent.addElement(type, key, 0);
    },
    
    addComplexElement: function(parent, key, xmlElement) {
        var sequence = this.queryXSD(xmlElement, "sequence");
        var complexElement = new policylib.Composite();


        if(sequence.length == 0) {
            return;
        }
        var children = sequence[0].childNodes;
        for(var i = 0; i < children.length; i++) {
            if(children[i]) {
                this.addElement(complexElement, children[i]);
            }
        }
        

        parent.addElement("composite", key, complexElement);
    },
    
    addAnnotation: function(parent, key, xmlElement) {
        var annotation = this.queryXSD(xmlElement, "annotation");
        if(annotation && annotation.length > 0) {
            var annotations = {};

            var docs = this.queryXSD(annotation[0], "documentation");

            if(docs) {
                for(var i = 0; i < docs.length; i++) {

                    var language = dojo.attr(docs[i], "xml:lang");
                    if(language && docs[i].firstChild) {
                        var content = docs[i].firstChild.data;
                                                                               
                        annotations[language] = content;
                    }

                }
            }
            parent.addAnnotation(key, annotations);
        }
    },


    addElementWithRestriction: function(parent, key, xmlElement) {

        var restriction = this.queryXSD(xmlElement, "restriction");
        if(!restriction || restriction.length == 0) return;

        var type = dojo.attr(restriction[0], "base");
        if(type) {
            type = type.replace("xsd:","");
        }
        
        var enumerations = this.queryXSD(restriction[0], "enumeration");

        if(enumerations && enumerations.length > 0) {

            var restrictionList = [];
            for (var i = 0; i < enumerations.length; i++) {
                restrictionList[i] = dojo.attr(enumerations[i], "value");
            }

            parent.addElementWithRestriction(type, key, restrictionList[0], restrictionList);
        }
        
        var minInclusive = this.queryXSD(xmlElement, "minInclusive");
        var maxInclusive = this.queryXSD(xmlElement, "maxInclusive");
        if(minInclusive && minInclusive.length > 0 && maxInclusive && 
            maxInclusive.length > 0) {
            var max = dojo.attr(maxInclusive[0], "value");
            var min = dojo.attr(minInclusive[0], "value");
            var type = type ? type : dojo.attr(xmlElement, "type").replace("xsd:", "");
            parent.addConstrainedElement(type, key, min, min, max);
        }
        
        var list = this.queryXSD(xmlElement, "list");
        var length = this.queryXSD(xmlElement, "length");
        if(list && list.length > 0 && length && length.length > 0 ) {
            var listType = dojo.attr(list[0], "itemType");
            listType = listType.replace("xsd:", "");
            var lengthValue = dojo.attr(length[0], "value") - 0;
            parent.addListElement(listType, key, lengthValue);
        }
    },
    
    // Private
    _elements  : ["element", "complexType"],
    
    _isRestricted : function(xmlElement) {
        var query = this.queryXSD(xmlElement, "restriction");
        return query && query.length > 0;
    }
});


dojo.declare("policylib.StateParser", policylib.XMLHelper, {
    constructor: function(policyLib) {
        this._policyLib = policyLib;
    },
    
    parseXML: function(xml) {

        var state = this.queryXSD(xml, "State");
        if(state && state.length > 0) {
            var elements = state[0].childNodes;
            for(var i = 0; i < elements.length; i++) {
                this.updateElement(this._policyLib, elements[i]);
            }
        }
    },
    
    updateElement: function(parent, xmlElement) {
 
        var nodeName = xmlElement.nodeName;
        if(nodeName == "#text") {
            return;
        }
        if(!parent.hasKey(nodeName)) {
            throw "Trying to update key " + nodeName + " but it's not in the Policy";
        }
        

        
        if(parent.getType(nodeName) == "composite") {
            this.updateCompositeElement(parent, xmlElement, nodeName);
        }
        else if(parent.isList(nodeName)) {
            this.updateListElement(parent, xmlElement, nodeName);
        }
        else {
            for(var i = 0; i < xmlElement.childNodes.length; i++) {
                if(xmlElement.childNodes[i].data) {
                    var value = dojo.trim(xmlElement.childNodes[i].data);
                    parent.updateElement(nodeName, value);
                }
            }
        }
    },
    
    updateCompositeElement: function(parent, xmlElement, nodeName) {
        var elements = xmlElement.childNodes;
        for(var i = 0; i < elements.length; i++) {
            this.updateElement(parent.getValue(nodeName), elements[i]);
        }
    },
    
    updateListElement: function(parent, xmlElement, nodeName) {
        var listUnparsed = xmlElement.firstChild.data;
        var list = listUnparsed.split(" ");
        parent.updateElement(nodeName, list);
    }
    
    
    
});


dojo.declare("policylib.GUIParser", policylib.XMLHelper, {
    constructor: function(policyLib) {
        this._policyLib = policyLib;
    },
   
    parseXML: function(xml) {
        var guiLayout = this.queryXSD(xml, "GuiLayout");
        if(guiLayout && guiLayout.length > 0) {
            var children = guiLayout[0].childNodes;
            for(var i = 0; i < children.length; i++) {
                if(this.isGUIElement(children[i])){
                    var gui = this.makeGUIElement(children[i]);
                    this._policyLib.setGUI(gui);
                    return;
                }
            }
        }
    },
   
    makeGUIElement: function(xml) {
        var nodeName = xml.nodeName;
        var element = null;
        if(this._keyValues.indexOf(nodeName) > -1) {
            element = this.makeKeyValue(nodeName, xml);
        }
        else if(this._containers.indexOf(nodeName) > -1) {
            element = this.makeContainer(nodeName, xml);
        }
        else if(nodeName == "PopupButton") {
            element = this.makePopupButton(xml);
        }
        else if(nodeName == "TabLayout") {
            element = this.makeTabLayout(xml);
        }
        else if(nodeName == "Grid") {
            element = this.makeGrid(xml);
        }
        else if(this._spaces.indexOf(nodeName) > -1) {
            element = this.makeSpace(nodeName, xml);
        }
        else if(nodeName == "Canvas") {
            element = this.makeCanvas(xml);
        }
        if(element) {
            this.addVisibilityInfo(element, xml);
        }
        return element;
    },
    
    makeSpace: function(type, xml) {
        var space = null;
        switch(type) {
            case 'VerticalSpace':
                space = new policylib.gui.VerticalSpace();
                break;
            case 'VerticalExpandingSpace':
                space = new policylib.gui.VerticalExpandingSpace();
                break;
            case 'HorizontalSpace':
                space = new policylib.gui.HorizontalSpace();
                break;
            case 'HorizontalExpandingSpace':
                space = new policylib.gui.HorizontalExpandingSpace();
                break;
        }
        return space;
    },
    
    makeGrid: function(xml) {
        var grid = new policylib.gui.Grid();
        var row = 0;
        var col = 0;
        for(var i = 0; i < xml.childNodes.length; i++) {
            if(this.isGUIElement(xml.childNodes[i])) {
                var rowElement = xml.childNodes[i];
                for(var j = 0; j < rowElement.childNodes.length; j++) {
                    if(this.isGUIElement(rowElement.childNodes[j])) {
                        var cellElement = rowElement.childNodes[j];
                        for(var k = 0; k < cellElement.childNodes.length; k++) {
                            if(this.isGUIElement(cellElement.childNodes[k])) {
                                grid.setChild(row, col, this.makeGUIElement(cellElement.childNodes[k]));
                            }
                        }
                        col++;
                    }
                }
                row++;
            }
            col = 0;
        }
        
        return grid;
    },
    
    makeContainer: function(type, xml) {
        var container = null;
        switch(type) {
            case 'HorizontalLayout':
                container = new policylib.gui.HorizontalLayout();
                break;
            case 'VerticalLayout':
                container = new policylib.gui.VerticalLayout();
                break;
            case 'ElementGroup':
                container  = new policylib.gui.ElementGroup(this.getKey(xml), this.showValue(xml));
                break;
        }
        
        for(var i = 0; i < xml.childNodes.length; i++) {
            if(this.isGUIElement(xml.childNodes[i])) {
                container.addChild(this.makeGUIElement(xml.childNodes[i]));
            }
        }
        return container;
    },
   
    makeKeyValue: function(type, xml) {
        var keyValue = new policylib.gui.KeyValue(type, this.getKey(xml), 
            this.showValue(xml));
        return keyValue;
                                                  
    },
    
    makeTabLayout: function(xml) {
        var tabLayout = new policylib.gui.TabLayout();
        
        for(var i = 0; i < xml.childNodes.length; i++) {
            if(this.isGUIElement(xml.childNodes[i])) {
                var tab = this.makeTab(xml.childNodes[i]);
                tabLayout.addChild(tab);
            }
        }
        return tabLayout;
    },
    makePopupButton: function(xml) {
        var button = new policylib.gui.PopupButton(this.getKey(xml), this.showValue(xml));
        for(var i = 0; i < xml.childNodes.length; i++) {
            if(this.isGUIElement(xml.childNodes[i])) {
                button.setChild(this.makeGUIElement(xml.childNodes[i]));
            }
        }
        return button;
    },
    makeTab: function(xml) {
        var tab = new policylib.gui.Tab(this.getKey(xml), this.showValue(xml));
        for(var i = 0; i < xml.childNodes.length; i++) {
            if(this.isGUIElement(xml.childNodes[i])) {
                tab.setChild(this.makeGUIElement(xml.childNodes[i]));
            }
        }
        return tab;
    },
    
    makeCanvas: function(xml) {
        return new policylib.gui.Canvas(this.getKey(xml), dojo.attr(xml, "renderlistKey"),
            dojo.attr(xml, "boundingboxKey"));
    },

    isGUIElement: function(xml) {
        return xml && xml.nodeName && xml.nodeName != "#text";
    },
    
    addVisibilityInfo: function(element, xml) {
        var visibilityKey = dojo.attr(xml, "visibilityKey");
        var visibilityKeyInverted = dojo.attr(xml, "visibilityKeyInverted");
        var enabledKey = dojo.attr(xml, "enabledKey");
        var enabledKeyInverted = dojo.attr(xml, "enabledKeyInverted");
       
        if(visibilityKey) {
            if(visibilityKeyInverted === undefined) {
                visibilityKeyInverted = false;
            }
            if(typeof visibilityKeyInverted =="string") {
                visibilityKeyInverted = (visibilityKeyInverted - 0) != 0;
            }
            visibilityKeyInverted = !! visibilityKeyInverted;
            element.setVisibilityKey(visibilityKey);
            element.setVisibilityKeyInverted(visibilityKeyInverted);
        } 
        if(enabledKey) {
            if(enabledKeyInverted === undefined) {
                enabledKeyInverted = false;
            }
            if(typeof enabledKeyInverted =="string") {
                enabledKeyInverted = (enabledKeyInverted - 0) != 0;
            }
            enabledKeyInverted = !! enabledKeyInverted;
            element.setEnabledKey(enabledKey);
            element.setEnabledKeyInverted(enabledKeyInverted);
        } 
    },
   
    getKey: function(xml) {
        return dojo.attr(xml, "key");
    },
   
    showValue: function(xml) {
        var showValue = dojo.attr(xml, "showValue");
        if(showValue === undefined || showValue == null || showValue.length == 0) {
            showValue = true;
        }
        if(typeof(showValue) =="string") {
            showValue = (showValue - 0) != 0;
        }
        showValue = !! showValue;
        
        return showValue;
    },
   
   
    // Private
    _containers : ["HorizontalLayout", "VerticalLayout", "ElementGroup"],
    _keyValues  : ["TextInput", "SpinBox", "DoubleSpinBox", "HorizontalSlider", 
    "VerticalSlider", "Checkbox", "RadioButtons", "Label", "ComboBox", "Button"],
    _spaces    : ["HorizontalSpace", "HorizontalExpandingSpace", "VerticalSpace",
    "VerticalExpandingSpace"]
   
   
});


