dojo.provide("test.policylib.PolicyLibTests");
dojo.require("policylib.PolicyLib");
dojo.require("policylib.PolicyParser");
dojo.require("policylib.gui.GUILayout");
dojo.require("policylib.PolicyBuilder");
dojo.require("policylib.StateParser");
dojo.require("dojox.xml.parser");
dojo.require("policylib.PolicySender");
dojo.require("policylib.PolicyReceiver");
dojo.require("dijit.layout.StackContainer");
dojo.require("3rdparty/webgl-utils");


dojo.require("gui.GUIBuilder");

var policyLib = new policylib.PolicyLib();

doh.register("PolicyLibTests",  {
    revisionTest: function() {
        var policy = new policylib.PolicyLib();
        doh.assertEqual(0, policy.getRevision());
        policy.setRevision(10);
        doh.assertEqual(10, policy.getRevision());
    },
    simpleAdd : function() {
        policyLib.addElement("string", "StringKey", "StringValue");

        doh.assertEqual("StringValue", policyLib.getValue("StringKey"));
    },
   
    simpleAnnotation: function() {
        policyLib.addElement("string", "key", "value");
        policyLib.addAnnotation("key", "A special key");
        doh.assertEqual("A special key", policyLib.getAnnotation("key"));
    },
   
    complexAnnotation: function() {
        policyLib.addElement("string", "ComplexAnnotation_1", "value");
        policyLib.addAnnotation("ComplexAnnotation_1", {
            "en": "Some text1"
        });

        
        policyLib.addElement("string", "ComplexAnnotation_2", "value");
        policyLib.addAnnotation("ComplexAnnotation_2", {
            "en": "Some text2"
        });
        doh.assertEqual("Some text2", policyLib.getAnnotation("ComplexAnnotation_2", "en"));
        doh.assertEqual("Some text1", policyLib.getAnnotation("ComplexAnnotation_1", "en"));
    },
   
    updateElement: function() {
        policyLib.addElement("integer", "myInt", 45);
        doh.assertEqual(45, policyLib.getValue("myInt"));
        policyLib.updateElement("myInt", 100);
        doh.assertEqual(100, policyLib.getValue("myInt"));
    },
   
    hasKey: function() {
        doh.assertFalse(policyLib.hasKey("randomKey"));
        policyLib.addElement("integer", "randomKey", 10);
        doh.assertTrue(policyLib.hasKey("randomKey"));
    },
   
    addElementWithRestriction: function() {
        policyLib.addElementWithRestriction("string", "RestrictedKey", "value", ["value", "someOtherValue"]);
       
        doh.assertTrue(policyLib.hasRestriction("RestrictedKey"));
        doh.assertEqual(["value", "someOtherValue"], policyLib.getRestriction("RestrictedKey"));                                    
    },
   
    addConstrainedElement: function() {
        policyLib.addConstrainedElement("integer", "somekey", 123, 0, 150);
        doh.assertEqual(123, policyLib.getValue("somekey"));
        doh.assertTrue(policyLib.isConstrained("somekey"));
        doh.assertEqual(150, policyLib.getMax("somekey"));
        doh.assertEqual(0, policyLib.getMin("somekey"));
    }
});

doh.register("ParserTest", {
    parseSimpleTest: function() {
        var parser = new policylib.PolicyParser(policyLib);

        dojo.xhrGet({
            url: "../../xml/getPolicyUpdate.xml",
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
        doh.assertEqual(10456, policyLib.getRevision());
        doh.assertTrue(policyLib.hasKey("mesh_0"));
        doh.assertTrue(policyLib.hasKey("boundingbox"));
        doh.assertTrue(policyLib.hasKey("viewer"));
        doh.assertTrue(policyLib.hasKey("timestep"));
        doh.assertTrue(policyLib.getValue("viewer").hasKey("Width"));
        doh.assertEqual(33, policyLib.getValue("timestep"));
        doh.assertTrue(policyLib.hasRestriction("timestep"));
        doh.assertEqual("Show mesh 2", policyLib.getAnnotation("mesh_2", "en"));
        doh.assertEqual("Show mesh 1", policyLib.getAnnotation("mesh_1", "en"));
        doh.assertEqual("Show mesh 0", policyLib.getAnnotation("mesh_0", "en"));
        doh.assertEqual("Configuration", policyLib.getAnnotation("showConfig", "en"));
        doh.assertEqual("Some text", policyLib.getValue("kindaAdvanced"));
        doh.assertEqual("Some other text", policyLib.getValue("alsoKindaAdvanced"));
        doh.assertEqual("Enable/show advanced dummy fields", policyLib.getAnnotation("showAdvanced", "en"));
        doh.assertTrue(policyLib.isConstrained("conInt"));
        doh.assertEqual(0, policyLib.getMin("conInt"));
        doh.assertEqual(100, policyLib.getMax("conInt"));
        
        var projection = [1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1.0202020406723022, -1,
        0, 0, -18.5638427734375, 0];
        doh.assertEqual(projection, policyLib.getValue("viewer").getValue("Projection"));
        
        doh.assertTrue(!!policyLib.GUI());
        doh.assertEqual("viewer", policyLib.GUI().child(1).key());
    },
   
    parseTwice: function() {
        var parser = new policylib.PolicyParser(policyLib);

        dojo.xhrGet({
            url: "../../xml/getPolicyUpdate.xml",
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
            url: "../../xml/getPolicyUpdate.xml",
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
        doh.assertTrue(policyLib.hasKey("mesh_0"));
        doh.assertTrue(policyLib.hasKey("boundingbox"));
        doh.assertTrue(policyLib.hasKey("viewer"));
        doh.assertTrue(policyLib.hasKey("timestep"));
        doh.assertTrue(policyLib.getValue("viewer").hasKey("Width"));
        doh.assertEqual(33, policyLib.getValue("timestep"));
        doh.assertTrue(policyLib.hasRestriction("timestep"));
        doh.assertEqual("Show mesh 2", policyLib.getAnnotation("mesh_2", "en"));
        doh.assertEqual("Show mesh 1", policyLib.getAnnotation("mesh_1", "en"));
        doh.assertEqual("Show mesh 0", policyLib.getAnnotation("mesh_0", "en"));
        doh.assertEqual("Configuration", policyLib.getAnnotation("showConfig", "en"));
        doh.assertEqual("Enable/show advanced dummy fields", policyLib.getAnnotation("showAdvanced", "en"));
        
        
        var projection = [1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1.0202020406723022, -1,
        0, 0, -18.5638427734375, 0];
        doh.assertEqual(projection, policyLib.getValue("viewer").getValue("Projection"));
        
        doh.assertTrue(!!policyLib.GUI());
        doh.assertEqual("viewer", policyLib.GUI().child(1).key());
    }
});


doh.register("GUITest", {
    testElement: function() {
        var element = new policylib.gui.Element();
        doh.assertFalse(element.visibilityKeyInverted());
        doh.assertFalse(element.enabledKeyInverted());
        
        var secondElement = new policylib.gui.Element();
        secondElement.setVisibilityKey("visibilitykey");
        doh.assertEqual("visibilitykey", secondElement.visibilityKey());
        
        secondElement.setVisibilityKey("somekey");
        doh.assertEqual("somekey", secondElement.visibilityKey());
    },
    
    
    testTab: function() {
        var tabContent = new policylib.gui.KeyValue("TextInput", "somekey");
        var tab  = new policylib.gui.Tab("someotherkey");
        tab.setChild(tabContent);
        
        doh.assertEqual(tabContent, tab.child());
        doh.assertFalse(tab.visibilityKeyInverted());
        doh.assertEqual("someotherkey", tab.key());
    }
    
    
    
});


doh.register("EventTest", {
    simpleEvent: function() {
        var pol = new policylib.PolicyLib();
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
        var pol = new policylib.PolicyLib();
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
        var pol = new policylib.PolicyLib();
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
        var policy = new policylib.PolicyLib();
        policy.addElement("bool", "key", false);
        doh.assertEqual(false, policy.getValue("key"));
        policy.addLocalListener("key", function(key, value) {
            valueSeen = value;
        });
        
        policy.updateElement("key", true);
        doh.assertEqual(true, valueSeen);
    }
});

doh.register("BuilderTest", {
    testSimpleBuild: function() {
        var policy = new policylib.PolicyLib();
        var builder = new policylib.PolicyBuilder(policy);
        var stateParser = new policylib.StateParser(policy);
        policy.addElement("string", "key", "sometext");
        doh.assertEqual("sometext", policy.getValue("key"));
        var xml = builder.buildXML();
        
        policy.updateElement("key", "someothertext");
        doh.assertEqual("someothertext", policy.getValue("key"));
        
        stateParser.parseXML(dojox.xml.parser.parse(xml));
        doh.assertEqual("sometext", policy.getValue("key"));
    },
    
    advancedWithAjax: function() {
        var policy = new policylib.PolicyLib();

        var parser = new policylib.PolicyParser(policy);
        var builder = new policylib.PolicyBuilder(policy);
        dojo.xhrGet({
            url: "../../xml/getPolicyUpdate.xml",
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
        doh.assertTrue(policy.hasKey("mesh_0"));
        doh.assertTrue(policy.hasKey("boundingbox"));
        doh.assertTrue(policy.hasKey("viewer"));
        doh.assertTrue(policy.hasKey("timestep"));
        doh.assertTrue(policy.getValue("viewer").hasKey("Width"));
        doh.assertEqual(33, policy.getValue("timestep"));
        doh.assertTrue(policy.hasRestriction("timestep"));
        doh.assertEqual("Show mesh 2", policy.getAnnotation("mesh_2", "en"));
        doh.assertEqual("Show mesh 1", policy.getAnnotation("mesh_1", "en"));
        doh.assertEqual("Show mesh 0", policy.getAnnotation("mesh_0", "en"));
        doh.assertEqual("Configuration", policy.getAnnotation("showConfig", "en"));
        doh.assertEqual("Enable/show advanced dummy fields", policy.getAnnotation("showAdvanced", "en"));
        
        var projection = [1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1.0202020406723022, -1,
        0, 0, -18.5638427734375, 0];
        doh.assertEqual(projection, policy.getValue("viewer").getValue("Projection"));
        
        doh.assertTrue(!!policy.GUI());
        doh.assertEqual("viewer", policy.GUI().child(1).key());
        
       
        var xml = dojox.xml.parser.parse(builder.buildXML());
        
        policy.updateElement("timestep", 10);
        doh.assertEqual(10, policy.getValue("timestep"));
        var stateParser = new policylib.StateParser(policy);
        
        stateParser.parseXML(xml);
        
        
        
        doh.assertTrue(policy.hasKey("mesh_0"));
        doh.assertTrue(policy.hasKey("boundingbox"));
        doh.assertTrue(policy.hasKey("viewer"));
        doh.assertTrue(policy.hasKey("timestep"));
        doh.assertTrue(policy.getValue("viewer").hasKey("Width"));
        doh.assertEqual(33, policy.getValue("timestep"));
        doh.assertTrue(policy.hasRestriction("timestep"));
        doh.assertEqual("Show mesh 2", policy.getAnnotation("mesh_2", "en"));
        doh.assertEqual("Show mesh 1", policy.getAnnotation("mesh_1", "en"));
        doh.assertEqual("Show mesh 0", policy.getAnnotation("mesh_0", "en"));
        doh.assertEqual("Configuration", policy.getAnnotation("showConfig", "en"));
        doh.assertEqual("Enable/show advanced dummy fields", policy.getAnnotation("showAdvanced", "en"));
        
      
        doh.assertEqual(projection, policy.getValue("viewer").getValue("Projection"));
    }
});

doh.register("PolicySenderTest", {
    simpleTest: function() {
        var policy = new policylib.PolicyLib();
        policy.addElement("integer", "timestep", 20);
        var sender = new policylib.PolicySender("../../xml/updateState.xml", policy);
        
        var deferred = new doh.Deferred();
        var started = false;
        var completed = false;
        dojo.subscribe("/policylib/updateSendStart", function() {
            started = true;
        });
        
        dojo.subscribe("/policylib/updateSendComplete", function() {
            if(!completed) {
                completed = true;
                doh.assertTrue(started);
                deferred.callback(true);
            }
        });
        
        policy.updateElement("timestep", 0);
        
    
        
        return deferred;
    }
});

doh.register("PolicyReceiverTest", {
    trivialTest: function() {
        var url = "../../xml/getPolicyUpdate.xml";
        var policy = new policylib.PolicyLib();
        var policyReceiver = new policylib.PolicyReceiver(url, policy);
        
        var deferred = new doh.Deferred();
        var complete = false;
        dojo.subscribe("/policylib/updateReceived", function() {
            doh.assertEqual(10456, policyLib.getRevision());
            doh.assertTrue(policyLib.hasKey("mesh_0"));
            doh.assertTrue(policyLib.hasKey("boundingbox"));
            doh.assertTrue(policyLib.hasKey("viewer"));
            doh.assertTrue(policyLib.hasKey("timestep"));
            doh.assertTrue(policyLib.getValue("viewer").hasKey("Width"));
            doh.assertEqual(33, policyLib.getValue("timestep"));
            doh.assertTrue(policyLib.hasRestriction("timestep"));
            doh.assertEqual("Show mesh 2", policyLib.getAnnotation("mesh_2", "en"));
            doh.assertEqual("Show mesh 1", policyLib.getAnnotation("mesh_1", "en"));
            doh.assertEqual("Show mesh 0", policyLib.getAnnotation("mesh_0", "en"));
            doh.assertEqual("Configuration", policyLib.getAnnotation("showConfig", "en"));
            doh.assertEqual("Enable/show advanced dummy fields", 
                policyLib.getAnnotation("showAdvanced", "en"));
        
            var projection = [1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, -1.0202020406723022, -1,
            0, 0, -18.5638427734375, 0];
            doh.assertEqual(projection, policyLib.getValue("viewer").getValue("Projection"));
        
            doh.assertTrue(!!policyLib.GUI());
            doh.assertEqual("viewer", policyLib.GUI().child(1).key());
            if(!complete) {
                deferred.callback(true);
                complete = true;
                policyReceiver.cancel();
            }
        });
        
        policyReceiver.longPoll();
        return deferred;
    }

});


doh.register("GUIBuilderTest", {
    simpleTest : function() {
        var policy = new policylib.PolicyLib();
        policy.addElement("string", "key", "value");
        var textInput = new policylib.gui.KeyValue("TextInput", "key");
        var contentPane = new dijit.layout.StackContainer();
        var builder = new gui.GUIBuilder(policy);
        contentPane.addChild(builder.buildGUI(textInput));
        doh.assertEqual("value", contentPane.getChildren()[0].getValue());
        
        policy.updateElement("key", "newvalue");
        doh.assertEqual("newvalue", contentPane.getChildren()[0].getValue());
        
    },
    
    simpleAnnotationTest : function() {
        var policy = new policylib.PolicyLib();
        policy.addElement("string", "key", "value");
        policy.addAnnotation("key", "Annotation");
        var textInput = new policylib.gui.KeyValue("TextInput", "key", false);
        var contentPane = new dijit.layout.StackContainer();
        var builder = new gui.GUIBuilder(policy);
        contentPane.addChild(builder.buildGUI(textInput));
        doh.assertEqual("Annotation", contentPane.getChildren()[0].getValue());
        
        policy.updateElement("key", "newvalue");
        doh.assertEqual("Annotation", contentPane.getChildren()[0].getValue());
    },
    
    horizontalLayoutTest : function() {
        var policy = new policylib.PolicyLib();
        policy.addElement("string", "key", "value");
        policy.addAnnotation("key", "Annotation");
        var textInput = new policylib.gui.KeyValue("TextInput", "key", false);
        var layout = new policylib.gui.HorizontalLayout();
        layout.addChild(textInput);
        var contentPane = new dijit.layout.StackContainer();
        var builder = new gui.GUIBuilder(policy);
        contentPane.addChild(builder.buildGUI(layout));
        //doh.assertEqual("Annotation", contentPane.getChildren()[0].getChildren()[0].getValue());
        
    },
    
    verticalLayoutTest : function() {
        var policy = new policylib.PolicyLib();
        policy.addElement("string", "key", "value");
        policy.addAnnotation("key", "Annotation");
        var textInput = new policylib.gui.KeyValue("TextInput", "key", false);
        var layout = new policylib.gui.VerticalLayout();
        layout.addChild(textInput);
        var contentPane = new dijit.layout.StackContainer();
        var builder = new gui.GUIBuilder(policy);
        contentPane.addChild(builder.buildGUI(layout));
        //doh.assertEqual("Annotation", contentPane.getChildren()[0].getChildren()[0].getValue());
        
    },
    
    
    gridLayoutTest : function() {
        var policy = new policylib.PolicyLib();
        policy.addElement("string", "key", "value");
        policy.addAnnotation("key", "Annotation");
        var textInput = new policylib.gui.KeyValue("TextInput", "key", false);
        var layout = new policylib.gui.Grid();
        layout.setChild(0,0, textInput);
        var contentPane = new dijit.layout.StackContainer();
        var builder = new gui.GUIBuilder(policy);
        contentPane.addChild(builder.buildGUI(layout));
        //doh.assertEqual("Annotation", contentPane.getChildren()[0].getChildren()[0].getValue());
        
    },
    
    guiFromXML : function() {
        var policy = new policylib.PolicyLib();

        var parser = new policylib.PolicyParser(policy);
        dojo.xhrGet({
            url: "../../xml/getPolicyUpdate.xml",
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
        var builder = new gui.GUIBuilder(policy);
        contentPane.addChild(builder.buildGUI(policy.GUI()));
        
    }
});