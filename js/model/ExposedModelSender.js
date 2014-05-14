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

dojo.provide("model.ExposedModelSender");
dojo.require("model.ExposedModelBuilder");

dojo.declare("model.ExposedModelSender", null, {
    constructor : function(urlHandler, modelLib) {
        this._urlHandler = urlHandler;
        this._modelLib = modelLib;
        this._modelLib.addListener(this.update, this);
        
        this._builder = new model.ExposedModelBuilder(modelLib);
        this._updateInProgress = false;
        this._pendingXML = false;
        this._keys = {};
        dojo.subscribe("/model/updateReceived", dojo.hitch(this, function() {
            this._parsingUpdate = true;
        }));
        
        dojo.subscribe("/model/updateParsed", dojo.hitch(this, function() {
            this._parsingUpdate = false;
        }));
    },
    
    
    
    update: function(key, value) {
        this._update(key);
    },
    _update: function(key) {
        if(this._parsingUpdate) {
            return;
        }
        if(key)
            this._keys[key] = true;
        if(this._updateInProgress) {
            this._pendingXML = true;
            return;
        }
        var xml = this._builder.buildXML(this._makeKeys());
        dojo.publish("/model/updateSendStart", [{"xml" : xml}]);
        this._send(xml);
    },
    
    _send: function(xml) {
        this._updateInProgress = true;
        // console.log("sending update");
        dojo.rawXhrPost({
            url: this._makeURL(),
            postData : xml,
            headers: {"Content-Type": "text/xml"},
            
            preventCache: true,
            load : dojo.hitch(this, function(response, ioArgs) {
                try {
                    this._updateComplete(response, ioArgs);
                } catch( error ) {
                    
                }
                return response;
            }),
            error: dojo.hitch(this, function(response, ioArgs) {
                this._updateError();
                dojo.publish("/model/updateSendError", [{"response": response, "ioArgs" : ioArgs}]);
                return response;
            })
            
        });
    },
    
    _updateComplete: function(response, ioArgs) {

        dojo.publish("/model/updateSendPartialComplete", [{"response": response, "ioArgs" : ioArgs}]);
        if(this._pendingXML) {
            var xmlBuild = this._builder.buildXML(this._makeKeys());
            this._pendingXML = false;
            this._send(xmlBuild);
        } else {
            this._updateInProgress = false;
            dojo.publish("/model/updateSendComplete", [{"response": response, "ioArgs" : ioArgs}]);
        }
    },
    
    _updateError: function() {
        this._updateInProgress = false;
        setTimeout(dojo.hitch(this, function() {this._update()}), 10);
    },
    
    
    _makeURL : function() {
        this._urlHandler.updateParams({"revision" : this._modelLib.getRevision(),
                                        "timestamp" : (new Date()).getTime()});
        return this._urlHandler.getURL();
    },
    
    _makeKeys : function() {
        var keys = [];
        for(var key in this._keys) {
            keys[keys.length] = key;
        }
        return keys;
    }
});


