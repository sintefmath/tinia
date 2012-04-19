/* 
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */


dojo.declare("policylib.URLHandler", null, {
    constructor: function(url, params) {
        this._url = "";
        if(params === undefined) {
            params = {};
        }
        this._params = params;
    },
    
    setURL: function(url) {
        this._url = url;
    },
    
    
    updateParams:  function(params) {
        for( var key in params ) {
            this._params[key] = params[key];
        }
    },
    
    getURL: function() {
        var url = this._url;
        if(url.indexOf("?") < 0 ) {
            url +="?"
        }
        else {
            url +="&";
        }
        
        for( var key in this._params) {
            url +=key+"="+this._params[key]+"&";
        }
        return url;
    }
})