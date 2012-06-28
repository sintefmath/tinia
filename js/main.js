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
