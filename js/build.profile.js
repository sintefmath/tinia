dependencies = {
    layers: [
	{
	    name: "policylib.js",
//	    resourceName: "policylib.layer",
	    layerDependencies: ["dijit", "dojox"],
	    dependencies: ["policylib.layer", "dojo", "dijit", "dojox"]
	},
	
    ],
    prefixes : [
	["dijit", "../dijit"],
	["dojox", "../dojox"],
	["policylib", "../policylib"],
	["gui", "../gui"],
	["3rdparty", "../3rdparty"],
	["renderlist", "../renderlist"]
    ]
};