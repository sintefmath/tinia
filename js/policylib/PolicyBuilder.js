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

