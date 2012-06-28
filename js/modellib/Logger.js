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

dojo.provide("model.Logger");
dojo.declare("model.Logger", null, {
    constructor: function(modelLib) {
        this._log = dojo.hitch(console, console.log);
        modelLib.addListener(function(key, value) {
            this._log("modelupdate: key=" + key+", value=", value);
        }, this);
        
        dojo.subscribe("/model/updateReceived", dojo.hitch(this, function(message) {
            this._log("updateReceived", message);
        }));
        dojo.subscribe("/model/updateSendStart", dojo.hitch(this, function(message) {
            this._log("updateSendStart", message);
        }));
        dojo.subscribe("/model/updateSendComplete", dojo.hitch(this, function(message) {
            this._log("updateSendComplete", message);
        }));
        dojo.subscribe("/model/updateSendError", dojo.hitch(this, function(message) {
            this._log("updateSendError", message);
        }));
    }
});