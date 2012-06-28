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


dojo.require("model.ExposedModel");
dojo.require("model.ExposedModelParser");
dojo.require("model.ExposedModelSender");
dojo.require("model.ExposedModelReceiver");
dojo.require("dijit.layout.BorderContainer");
dojo.require("gui.GUIBuilder");
dojo.require("model.Logger");
dojo.require("model.URLHandler");

function run(getExposedModelUpdateURL, updateStateURL, renderlistURL, isLocal, debug, renderList) {
    var model = new model.ExposedModel();
    var parser = new model.ExposedModelParser(model);
    dojo.xhrGet({
        url: getExposedModelUpdateURL,
        handleAs: "xml",
        sync: false,
        load: function(result, ioArgs) {
            console.log(ioArgs);
            dojo.byId("gui").innerHTML ="";
            parser.parseXML(result);

            var modelSenderUrlHandler = new model.URLHandler(updateStateURL);
            var builder = new gui.GUIBuilder(model, renderlistURL, isLocal,
                                             modelSenderUrlHandler, renderList);

            var appLayout = new dijit.layout.BorderContainer({
                design: "headline",
                style: "width: 100%; height: 100%"
            }, dojo.byId("gui"));

            if(!isLocal)
                var sender = new model.ExposedModelSender(modelSenderUrlHandler, model);
            var mainWindow = builder.buildGUI(model.GUI(), modelSenderUrlHandler);


	    
            mainWindow.set("region", "center");
            appLayout.addChild(mainWindow);

	    
            //     appLayout.startup();
            mainWindow.startup();

            
            var receiver = new model.ExposedModelReceiver(getExposedModelUpdateURL, model);
            if(!isLocal) {
                receiver.longPoll();
            }

            if(debug) {
                new model.Logger(model);
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
