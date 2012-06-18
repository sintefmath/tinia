
dojo.require("policylib.PolicyLib");
dojo.require("policylib.PolicyParser");
dojo.require("policylib.PolicySender");
dojo.require("policylib.PolicyReceiver");
dojo.require("dijit.layout.BorderContainer");
dojo.require("gui.GUIBuilder");
dojo.require("policylib.Logger");
dojo.require("policylib.URLHandler");

function run(getPolicyUpdateURL, updateStateURL, renderlistURL, isLocal, debug, renderList) {
    var policy = new policylib.PolicyLib();
    var parser = new policylib.PolicyParser(policy);
    dojo.xhrGet({
        url: getPolicyUpdateURL,
        handleAs: "xml",
        sync: false,
        load: function(result, ioArgs) {
            console.log(ioArgs);
            dojo.byId("gui").innerHTML ="";
            parser.parseXML(result);

            var policySenderUrlHandler = new policylib.URLHandler(updateStateURL);
            var builder = new gui.GUIBuilder(policy, renderlistURL, isLocal,
                                             policySenderUrlHandler, renderList);

            var appLayout = new dijit.layout.BorderContainer({
                design: "headline",
                style: "width: 100%; height: 100%"
            }, dojo.byId("gui"));

            if(!isLocal)
                var sender = new policylib.PolicySender(policySenderUrlHandler, policy);
            var mainWindow = builder.buildGUI(policy.GUI(), policySenderUrlHandler);


	    
            mainWindow.set("region", "center");
            appLayout.addChild(mainWindow);

	    
            //     appLayout.startup();
            mainWindow.startup();

            
            var receiver = new policylib.PolicyReceiver(getPolicyUpdateURL, policy);
            if(!isLocal) {
                receiver.longPoll();
            }

            if(debug) {
                new policylib.Logger(policy);
            }
            console.debug( "BAA" );
            return result;
        },
        error: function(result, ioArgs) {
            console.log(result);
            return result;
        }
    });



}
