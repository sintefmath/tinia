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