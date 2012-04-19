dojo.require("policylib.PolicyParser");
dojo.provide("policylib.PolicyReceiver");

dojo.declare("policylib.PolicyReceiver", null, {
    constructor: function(url, policyLib) {
        this._policyLib = policyLib;
        this._url = url;
        this._parser = new policylib.PolicyParser(policyLib);
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
            timeout: 200000,
            preventCache: true,
            
            content: {
                revision: this._policyLib.getRevision()
                },
                
            handleAs: "xml",
            
            failOk : true,
            
            load : dojo.hitch(this, function(response, ioArgs) {
                if(!response) {
                    this._postUpdate(0);
                    return response;
                }
                if(response.dojoType && response.dojoType == "timeout" || (response.xhr && response.xhr.status == 408)) {
                    this._postUpdate(0);
                    return response;
                }
                dojo.publish("/policylib/updateReceived", [{"response": response,
                                                            "ioArgs": ioArgs}]);
                this._handleUpdate(response);
                dojo.publish("/policylib/updateParsed", [{"response": response,
                                                            "ioArgs": ioArgs}]);
                this._postUpdate(0);
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

