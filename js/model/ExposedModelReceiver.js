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

dojo.require("model.ExposedModelParser");
dojo.provide("model.ExposedModelReceiver");

dojo.declare("model.ExposedModelReceiver", null, {
    constructor: function(url, modelLib) {
        this._modelLib = modelLib;
        this._url = url;
        this._parser = new model.ExposedModelParser(modelLib);
        this._cancel = false;
    },
   
    cancel: function() {
        this._cancel = true;
    },
   
    longPoll : function() {
        this._cancel = false;
        this._longPoll();
    },
    _longPoll : function() {
        dojo.xhrGet({
            url: this._url,
            timeout: 0,
            preventCache: true,
            
            content: {
                revision: this._modelLib.getRevision()
                },
                
            handleAs: "xml",
            
            failOk : true,
            
            load : dojo.hitch(this, function(response, ioArgs) {


                if(!response) {
                    //this._longPoll();
                    this._postUpdate(0);
                    return response;
                }
                if(response.dojoType && response.dojoType == "timeout" || (response.xhr && response.xhr.status == 408)) {
                    //this._longPoll();
                  this._postUpdate(0);
                    return response;
                }
                
                console.log("update received");
                dojo.publish("/model/updateReceived", [{"response": response,
                                                            "ioArgs": ioArgs}]);
                this._handleUpdate(response);
                dojo.publish("/model/updateParsed", [{"response": response,
                                                            "ioArgs": ioArgs}]);
                                                    
                this._postUpdate(0);
                //this._longPoll();
                return response; 
            }),
           
            error : dojo.hitch(this, function(response, ioArgs) {
                this._postUpdate(0);
                return response;
            })
        });
    },
    
    _handleUpdate: function(response) {
        this._parser.parseXML(response);
    },
    
    
    _postUpdate: function(timeout) {
        if(!this._cancel) {
            setTimeout(dojo.hitch(this, this._longPoll), timeout);
        }
    }
});

