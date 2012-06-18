dojo.provide("policylib.PolicySender");
dojo.require("policylib.PolicyBuilder");

dojo.declare("policylib.PolicySender", null, {
    constructor : function(urlHandler, policyLib) {
        this._urlHandler = urlHandler;
        this._policyLib = policyLib;
        this._policyLib.addListener(this.update, this);
        
        this._builder = new policylib.PolicyBuilder(policyLib);
        this._updateInProgress = false;
        this._pendingXML = false;
        this._keys = {};
        dojo.subscribe("/policylib/updateReceived", dojo.hitch(this, function() {
            this._parsingUpdate = true;
        }));
        
        dojo.subscribe("/policylib/updateParsed", dojo.hitch(this, function() {
            this._parsingUpdate = false;
            if(this._repostUpdate) {
                this._update();
            }
            this._repostUpdate = false;

        }));
    },
    
    
    
    update: function(key, value) {
        this._update(key);
    },
    _update: function(key) {
        if(this._parsingUpdate) {
            this._repostUpdate = true;
            if (key) this._keys[key] = true;
            return;
        }
        if(key)
            this._keys[key] = true;
        if(this._updateInProgress) {
            this._pendingXML = true;
            return;
        }
        var xml = this._builder.buildXML(this._makeKeys());
        dojo.publish("/policylib/updateSendStart", [{"xml" : xml}]);
        this._send(xml);
    },
    
    _send: function(xml) {
        this._updateInProgress = true;
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
                dojo.publish("/policylib/updateSendError", [{"response": response, "ioArgs" : ioArgs}]);
                return response;
            })
            
        });
    },
    
    _updateComplete: function(response, ioArgs) {

        dojo.publish("/policylib/updateSendPartialComplete", [{"response": response, "ioArgs" : ioArgs}]);
        if(this._pendingXML) {
            var xmlBuild = this._builder.buildXML(this._makeKeys());
            this._pendingXML = false;
            this._send(xmlBuild);
        } else {
            this._updateInProgress = false;
            dojo.publish("/policylib/updateSendComplete", [{"response": response, "ioArgs" : ioArgs}]);
        }
    },
    
    _updateError: function() {
        this._updateInProgress = false;
        setTimeout(dojo.hitch(this, function() {this._update()}), 10);
    },
    
    
    _makeURL : function() {
        this._urlHandler.updateParams({"revision" : this._policyLib.getRevision(),
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


