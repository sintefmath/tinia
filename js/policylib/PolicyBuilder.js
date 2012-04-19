dojo.provide("policylib.PolicyBuilder");

dojo.declare("policylib.PolicyBuilder", null, {
   constructor: function(policyLib) {
       this._policyLib = policyLib;
   },
   
   buildXML : function(keys) {
        var xml = "<State>\n";
        if(!keys) {
            keys = this._policyLib.keys();
        }
        for(var i = 0; i < keys.length; i++) {
            xml+=this._makeXML(keys[i], this._policyLib);
        }
        xml +="</State>\n";
        return xml;
   },
   
   _makeXML : function(key, parent) {
       var xml = "<" + key + ">";
       if(parent.getType(key) == "composite") {
           xml+="\n";
           var keys = parent.getValue(key).keys();
           for(var i = 0; i < keys.length; i++) {
               xml += this._makeXML(keys[i], parent.getValue(key));
           }
       }
       else if(parent.isList(key)) {
           var list = parent.getValue(key);
           for(var i = 0; i < list.length; i++) {
               xml += list[i] + " "
           }
           xml = xml.substring(0, xml.length-1);
       }
       else if(parent.getType(key) == "bool") {
           xml += parent.getValue(key) ? "1" : "0"; 
       }
       else {
           xml += this._sanify(parent.getValue(key));
       }
       xml += "</" + key + ">\n";
       return xml;
   },
   
   _sanify: function(text) {
       if(typeof(text) != "string") return text;
       return text.replace("<","").replace(">", "");
   }
   
});

