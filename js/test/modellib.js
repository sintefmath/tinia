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

dojo.provide("test.model.ExposedModelTests");
dojo.require("model.ExposedModel");
dojo.require("model.ExposedModelParser");
dojo.require("model.gui.GUILayout");
dojo.require("model.ExposedModelBuilder");
dojo.require("model.StateParser");
dojo.require("dojox.xml.parser");
dojo.require("model.ExposedModelSender");
dojo.require("model.ExposedModelReceiver");
dojo.require("model.URLHandler");
dojo.require("dijit.layout.StackContainer");
dojo.require("3rdparty/webgl-utils");
dojo.require("3rdparty.glMatrix");


dojo.require("gui.GUIBuilder");

var modelLib = new model.ExposedModel();

doh.register("ExposedModelTests",  {
    revisionTest: function() {
        var model = new model.ExposedModel();
        doh.assertEqual(0, model.getRevision());
        model.setRevision(10);
        doh.assertEqual(10, model.getRevision());
    },
    simpleAdd : function() {
        modelLib.addElement("string", "StringKey", "StringValue");

        doh.assertEqual("StringValue", modelLib.getValue("StringKey"));
    },
   
    simpleAnnotation: function() {
        modelLib.addElement("string", "key", "value");
        modelLib.addAnnotation("key", "A special key");
        doh.assertEqual("A special key", modelLib.getAnnotation("key"));
    },
   
    complexAnnotation: function() {
        modelLib.addElement("string", "ComplexAnnotation_1", "value");
        modelLib.addAnnotation("ComplexAnnotation_1", {
            "en": "Some text1"
        });

        
        modelLib.addElement("string", "ComplexAnnotation_2", "value");
        modelLib.addAnnotation("ComplexAnnotation_2", {
            "en": "Some text2"
        });
        doh.assertEqual("Some text2", modelLib.getAnnotation("ComplexAnnotation_2", "en"));
        doh.assertEqual("Some text1", modelLib.getAnnotation("ComplexAnnotation_1", "en"));
    },
   
    updateElement: function() {
        modelLib.addElement("integer", "myInt", 45);
        doh.assertEqual(45, modelLib.getValue("myInt"));
        modelLib.updateElement("myInt", 100);
        doh.assertEqual(100, modelLib.getValue("myInt"));
    },
   
    hasKey: function() {
        doh.assertFalse(modelLib.hasKey("randomKey"));
        modelLib.addElement("integer", "randomKey", 10);
        doh.assertTrue(modelLib.hasKey("randomKey"));
    },
   
    addElementWithRestriction: function() {
        modelLib.addElementWithRestriction("string", "RestrictedKey", "value", ["value", "someOtherValue"]);
       
        doh.assertTrue(modelLib.hasRestriction("RestrictedKey"));
        doh.assertEqual(["value", "someOtherValue"], modelLib.getRestriction("RestrictedKey"));                                    
    },
   
    addConstrainedElement: function() {
        modelLib.addConstrainedElement("integer", "somekey", 123, 0, 150);
        doh.assertEqual(123, modelLib.getValue("somekey"));
        doh.assertTrue(modelLib.isConstrained("somekey"));
        doh.assertEqual(150, modelLib.getMax("somekey"));
        doh.assertEqual(0, modelLib.getMin("somekey"));
    }
});

doh.register("ParserTest", {
    parseSimpleTest: function() {
        var parser = new model.ExposedModelParser(modelLib);

        dojo.xhrGet({
            url: "../../xml/getExposedModelUpdate.xml",
            handleAs: "xml",
            sync: true,
            load: function(result, ioArgs) {
                parser.parseXML(result);
                return result;

                
            },
            error: function(result, ioArgs) {
                console.log(result);
                return result;
            }
        });
        doh.assertEqual(10456, modelLib.getRevision());
        doh.assertTrue(modelLib.hasKey("mesh_0"));
        doh.assertTrue(modelLib.hasKey("boundingbox"));
        doh.assertTrue(modelLib.hasKey("viewer"));
        doh.assertTrue(modelLib.hasKey("timestep"));
        doh.assertTrue(modelLib.getValue("viewer").hasKey("Width"));
        doh.assertEqual(33, modelLib.getValue("timestep"));
        doh.assertTrue(modelLib.hasRestriction("timestep"));
        doh.assertEqual("Show mesh 2", modelLib.getAnnotation("mesh_2", "en"));
        doh.assertEqual("Show mesh 1", modelLib.getAnnotation("mesh_1", "en"));
        doh.assertEqual("Show mesh 0", modelLib.getAnnotation("mesh_0", "en"));
        doh.assertEqual("Configuration", modelLib.getAnnotation("showConfig", "en"));
        doh.assertEqual("Some text", modelLib.getValue("kindaAdvanced"));
        doh.assertEqual("Some other text", modelLib.getValue("alsoKindaAdvanced"));
        doh.assertEqual("Enable/show advanced dummy fields", modelLib.getAnnotation("showAdvanced", "en"));
        doh.assertTrue(modelLib.isConstrained("conInt"));
        doh.assertEqual(0, modelLib.getMin("conInt"));
        doh.assertEqual(100, modelLib.getMax("conInt"));
        
        var projection = [1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1.0202020406723022, -1,
        0, 0, -18.5638427734375, 0];
        doh.assertEqual(projection, modelLib.getValue("viewer").getValue("Projection"));
        
        doh.assertTrue(!!modelLib.GUI());

        doh.assertEqual("viewer", modelLib.GUI().child(1).child(0,0).key());
    },
   
    parseTwice: function() {
        var parser = new model.ExposedModelParser(modelLib);

        dojo.xhrGet({
            url: "../../xml/getExposedModelUpdate.xml",
            handleAs: "xml",
            sync: true,
            load: function(result, ioArgs) {
                parser.parseXML(result);
                return result;
            },
            error: function(result, ioArgs) {
                console.log(result);
                return result;
            }
        });

        dojo.xhrGet({
            url: "../../xml/getExposedModelUpdate.xml",
            handleAs: "xml",
            sync: true,
            load: function(result, ioArgs) {
                parser.parseXML(result);
                return result;
            },
            error: function(result, ioArgs) {
                console.log(result);
                return result;
            }
        });
        doh.assertTrue(modelLib.hasKey("mesh_0"));
        doh.assertTrue(modelLib.hasKey("boundingbox"));
        doh.assertTrue(modelLib.hasKey("viewer"));
        doh.assertTrue(modelLib.hasKey("timestep"));
        doh.assertTrue(modelLib.getValue("viewer").hasKey("Width"));
        doh.assertEqual(33, modelLib.getValue("timestep"));
        doh.assertTrue(modelLib.hasRestriction("timestep"));
        doh.assertEqual("Show mesh 2", modelLib.getAnnotation("mesh_2", "en"));
        doh.assertEqual("Show mesh 1", modelLib.getAnnotation("mesh_1", "en"));
        doh.assertEqual("Show mesh 0", modelLib.getAnnotation("mesh_0", "en"));
        doh.assertEqual("Configuration", modelLib.getAnnotation("showConfig", "en"));
        doh.assertEqual("Enable/show advanced dummy fields", modelLib.getAnnotation("showAdvanced", "en"));
        
        
        var projection = [1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1.0202020406723022, -1,
        0, 0, -18.5638427734375, 0];
        doh.assertEqual(projection, modelLib.getValue("viewer").getValue("Projection"));
        
        doh.assertTrue(!!modelLib.GUI());
        doh.assertEqual("viewer", modelLib.GUI().child(1).child(0,0).key());
    }
});


doh.register("GUITest", {
    testElement: function() {
        var element = new model.gui.Element();
        doh.assertFalse(element.visibilityKeyInverted());
        doh.assertFalse(element.enabledKeyInverted());
        
        var secondElement = new model.gui.Element();
        secondElement.setVisibilityKey("visibilitykey");
        doh.assertEqual("visibilitykey", secondElement.visibilityKey());
        
        secondElement.setVisibilityKey("somekey");
        doh.assertEqual("somekey", secondElement.visibilityKey());
    },
    
    
    testTab: function() {
        var tabContent = new model.gui.KeyValue("TextInput", "somekey");
        var tab  = new model.gui.Tab("someotherkey");
        tab.setChild(tabContent);
        
        doh.assertEqual(tabContent, tab.child());
        doh.assertFalse(tab.visibilityKeyInverted());
        doh.assertEqual("someotherkey", tab.key());
    }
    
    
    
});


doh.register("EventTest", {
    simpleEvent: function() {
        var pol = new model.ExposedModel();
        pol.addElement("string", "key", "sometext");
        var called = false;
        pol.addLocalListener("key", function() {
            called = true;
        });
        pol.updateElement("key", "sometext");
        doh.assertFalse(called);
        pol.updateElement("key", "someothertext");
        doh.assertTrue(called);
    },
    
    contextEvent: function() {
        var context = {
            someValue : false
        };
        var pol = new model.ExposedModel();
        pol.addElement("string", "key", "sometext");
        pol.addLocalListener("key", function() {
            context.someValue = true;
        },
        context);
        pol.updateElement("key", "sometext");
        doh.assertFalse(context.someValue);
        pol.updateElement("key", "someothertext");
        doh.assertTrue(context.someValue);
    },
    contextGlobalEvent: function() {
        var context = {
            someValue : false
        };
        var pol = new model.ExposedModel();
        pol.addElement("string", "key", "sometext");
        pol.addListener(function() {
            context.someValue = true;
        },
        context);
        pol.updateElement("key", "sometext");
        doh.assertFalse(context.someValue);
        pol.updateElement("key", "someothertext");
        doh.assertTrue(context.someValue);
    },
    
    valueChangedEvent: function() {
        var valueSeen = false;
        var model = new model.ExposedModel();
        model.addElement("bool", "key", false);
        doh.assertEqual(false, model.getValue("key"));
        model.addLocalListener("key", function(key, value) {
            valueSeen = value;
        });
        
        model.updateElement("key", true);
        doh.assertEqual(true, valueSeen);
    }
});

doh.register("BuilderTest", {
    testSimpleBuild: function() {
        var model = new model.ExposedModel();
        var builder = new model.ExposedModelBuilder(model);
        var stateParser = new model.StateParser(model);
        model.addElement("string", "key", "sometext");
        doh.assertEqual("sometext", model.getValue("key"));
        var xml = builder.buildXML();
        
        model.updateElement("key", "someothertext");
        doh.assertEqual("someothertext", model.getValue("key"));
        
        stateParser.parseXML(dojox.xml.parser.parse(xml));
        doh.assertEqual("sometext", model.getValue("key"));
    },
    
    advancedWithAjax: function() {
        var model = new model.ExposedModel();

        var parser = new model.ExposedModelParser(model);
        var builder = new model.ExposedModelBuilder(model);
        dojo.xhrGet({
            url: "../../xml/getExposedModelUpdate.xml",
            handleAs: "xml",
            sync: true,
            load: function(result, ioArgs) {
                parser.parseXML(result);
                return result;
            },
            error: function(result, ioArgs) {
                console.log(result);
                return result;
            }
        });
        doh.assertTrue(model.hasKey("mesh_0"));
        doh.assertTrue(model.hasKey("boundingbox"));
        doh.assertTrue(model.hasKey("viewer"));
        doh.assertTrue(model.hasKey("timestep"));
        doh.assertTrue(model.getValue("viewer").hasKey("Width"));
        doh.assertEqual(33, model.getValue("timestep"));
        doh.assertTrue(model.hasRestriction("timestep"));
        doh.assertEqual("Show mesh 2", model.getAnnotation("mesh_2", "en"));
        doh.assertEqual("Show mesh 1", model.getAnnotation("mesh_1", "en"));
        doh.assertEqual("Show mesh 0", model.getAnnotation("mesh_0", "en"));
        doh.assertEqual("Configuration", model.getAnnotation("showConfig", "en"));
        doh.assertEqual("Enable/show advanced dummy fields", model.getAnnotation("showAdvanced", "en"));
        
        var projection = [1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1.0202020406723022, -1,
        0, 0, -18.5638427734375, 0];
        doh.assertEqual(projection, model.getValue("viewer").getValue("Projection"));
        
        doh.assertTrue(!!model.GUI());
        doh.assertEqual("viewer", model.GUI().child(1).child(0,0).key());
        
       
        var xml = dojox.xml.parser.parse(builder.buildXML());
        
        model.updateElement("timestep", 10);
        doh.assertEqual(10, model.getValue("timestep"));
        var stateParser = new model.StateParser(model);
        
        stateParser.parseXML(xml);
        
        
        
        doh.assertTrue(model.hasKey("mesh_0"));
        doh.assertTrue(model.hasKey("boundingbox"));
        doh.assertTrue(model.hasKey("viewer"));
        doh.assertTrue(model.hasKey("timestep"));
        doh.assertTrue(model.getValue("viewer").hasKey("Width"));
        doh.assertEqual(33, model.getValue("timestep"));
        doh.assertTrue(model.hasRestriction("timestep"));
        doh.assertEqual("Show mesh 2", model.getAnnotation("mesh_2", "en"));
        doh.assertEqual("Show mesh 1", model.getAnnotation("mesh_1", "en"));
        doh.assertEqual("Show mesh 0", model.getAnnotation("mesh_0", "en"));
        doh.assertEqual("Configuration", model.getAnnotation("showConfig", "en"));
        doh.assertEqual("Enable/show advanced dummy fields", model.getAnnotation("showAdvanced", "en"));
        
      
        doh.assertEqual(projection, model.getValue("viewer").getValue("Projection"));
    }
});

doh.register("ExposedModelSenderTest", {
    simpleTest: function() {
        var model = new model.ExposedModel();
        model.addElement("integer", "timestep", 20);
        var modelSenderUrlHandler = new model.URLHandler("../../xml/updateState.xml");
        var sender = new model.ExposedModelSender(modelSenderUrlHandler, model);
        
        var deferred = new doh.Deferred();
        var started = false;
        var completed = false;
        dojo.subscribe("/model/updateSendStart", function() {
            started = true;
        });
        
        dojo.subscribe("/model/updateSendComplete", function() {
            if(!completed) {
                completed = true;
                doh.assertTrue(started);
                deferred.callback(true);
            }
        });
        
        model.updateElement("timestep", 0);
        
    
        
        return deferred;
    }
});

doh.register("ExposedModelReceiverTest", {
    trivialTest: function() {
        var url = "../../xml/getExposedModelUpdate.xml";
        var model = new model.ExposedModel();
        var modelReceiver = new model.ExposedModelReceiver(url, model);
        
        var deferred = new doh.Deferred();
        var complete = false;
        dojo.subscribe("/model/updateReceived", function() {
            doh.assertEqual(10456, modelLib.getRevision());
            doh.assertTrue(modelLib.hasKey("mesh_0"));
            doh.assertTrue(modelLib.hasKey("boundingbox"));
            doh.assertTrue(modelLib.hasKey("viewer"));
            doh.assertTrue(modelLib.hasKey("timestep"));
            doh.assertTrue(modelLib.getValue("viewer").hasKey("Width"));
            doh.assertEqual(33, modelLib.getValue("timestep"));
            doh.assertTrue(modelLib.hasRestriction("timestep"));
            doh.assertEqual("Show mesh 2", modelLib.getAnnotation("mesh_2", "en"));
            doh.assertEqual("Show mesh 1", modelLib.getAnnotation("mesh_1", "en"));
            doh.assertEqual("Show mesh 0", modelLib.getAnnotation("mesh_0", "en"));
            doh.assertEqual("Configuration", modelLib.getAnnotation("showConfig", "en"));
            doh.assertEqual("Enable/show advanced dummy fields", 
                modelLib.getAnnotation("showAdvanced", "en"));
        
            var projection = [1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, -1.0202020406723022, -1,
            0, 0, -18.5638427734375, 0];
            doh.assertEqual(projection, modelLib.getValue("viewer").getValue("Projection"));
        
            doh.assertTrue(!!modelLib.GUI());
            doh.assertEqual("viewer", modelLib.GUI().child(1).child(0,0).key());
            if(!complete) {
                deferred.callback(true);
                complete = true;
                modelReceiver.cancel();
            }
        });
        
        modelReceiver.longPoll();
        return deferred;
    }

});


doh.register("GUIBuilderTest", {
    simpleTest : function() {
        var model = new model.ExposedModel();
        model.addElement("string", "key", "value");
        var textInput = new model.gui.KeyValue("TextInput", "key");
        var contentPane = new dijit.layout.StackContainer();
        var builder = new gui.GUIBuilder(model);
        contentPane.addChild(builder.buildGUI(textInput));
        doh.assertEqual("value", contentPane.getChildren()[0].getValue());
        
        model.updateElement("key", "newvalue");
        doh.assertEqual("newvalue", contentPane.getChildren()[0].getValue());
        
    },
    
    simpleAnnotationTest : function() {
        var model = new model.ExposedModel();
        model.addElement("string", "key", "value");
        model.addAnnotation("key", "Annotation");
        var textInput = new model.gui.KeyValue("TextInput", "key", false);
        var contentPane = new dijit.layout.StackContainer();
        var builder = new gui.GUIBuilder(model);
        contentPane.addChild(builder.buildGUI(textInput));
        doh.assertEqual("Annotation", contentPane.getChildren()[0].getValue());
        
        model.updateElement("key", "newvalue");
        doh.assertEqual("Annotation", contentPane.getChildren()[0].getValue());
    },
    
    horizontalLayoutTest : function() {
        var model = new model.ExposedModel();
        model.addElement("string", "key", "value");
        model.addAnnotation("key", "Annotation");
        var textInput = new model.gui.KeyValue("TextInput", "key", false);
        var layout = new model.gui.HorizontalLayout();
        layout.addChild(textInput);
        var contentPane = new dijit.layout.StackContainer();
        var builder = new gui.GUIBuilder(model);
        contentPane.addChild(builder.buildGUI(layout));
        //doh.assertEqual("Annotation", contentPane.getChildren()[0].getChildren()[0].getValue());
        
    },
    
    verticalLayoutTest : function() {
        var model = new model.ExposedModel();
        model.addElement("string", "key", "value");
        model.addAnnotation("key", "Annotation");
        var textInput = new model.gui.KeyValue("TextInput", "key", false);
        var layout = new model.gui.VerticalLayout();
        layout.addChild(textInput);
        var contentPane = new dijit.layout.StackContainer();
        var builder = new gui.GUIBuilder(model);
        contentPane.addChild(builder.buildGUI(layout));
        //doh.assertEqual("Annotation", contentPane.getChildren()[0].getChildren()[0].getValue());
        
    },
    
    
    gridLayoutTest : function() {
        var modelSenderUrlHandler = new model.URLHandler("../../xml/updateState.xml");
        
        var model = new model.ExposedModel();
        model.addElement("string", "key", "value");
        model.addAnnotation("key", "Annotation");
        var textInput = new model.gui.KeyValue("TextInput", "key", false);
        var layout = new model.gui.Grid();
        layout.setChild(0,0, textInput);
        var contentPane = new dijit.layout.StackContainer();
        var builder = new gui.GUIBuilder(model, "", true, modelSenderUrlHandler, "");
        contentPane.addChild(builder.buildGUI(layout));
        //doh.assertEqual("Annotation", contentPane.getChildren()[0].getChildren()[0].getValue());
        
    },
    
    guiFromXML : function() {
        var model = new model.ExposedModel();
        var modelSenderUrlHandler = new model.URLHandler("../../xml/updateState.xml");
        
        var parser = new model.ExposedModelParser(model);
        dojo.xhrGet({
            url: "../../xml/getExposedModelUpdate.xml",
            handleAs: "xml",
            sync: true,
            load: function(result, ioArgs) {
                parser.parseXML(result);
                return result;
            },
            error: function(response, ioArgs) {
                console.log(response);
                return response;
            }
        });
        var contentPane = new dijit.layout.StackContainer();
        var builder = new gui.GUIBuilder(model, "", true, modelSenderUrlHandler, "");
        contentPane.addChild(builder.buildGUI(model.GUI()));
        
    }
});