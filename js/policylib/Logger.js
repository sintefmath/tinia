dojo.provide("policylib.Logger");
dojo.declare("policylib.Logger", null, {
    constructor: function(policyLib) {
        this._log = dojo.hitch(console, console.log);
        policyLib.addListener(function(key, value) {
            this._log("policyupdate: key=" + key+", value=", value);
        }, this);
        
        dojo.subscribe("/policylib/updateReceived", dojo.hitch(this, function(message) {
            this._log("updateReceived", message);
        }));
        dojo.subscribe("/policylib/updateSendStart", dojo.hitch(this, function(message) {
            this._log("updateSendStart", message);
        }));
        dojo.subscribe("/policylib/updateSendComplete", dojo.hitch(this, function(message) {
            this._log("updateSendComplete", message);
        }));
        dojo.subscribe("/policylib/updateSendError", dojo.hitch(this, function(message) {
            this._log("updateSendError", message);
        }));
    }
});