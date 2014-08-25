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

dojo.declare("gui.ImageReceiver", null, {
    constructor : function(imageElement, url, type) {
        this._imageElement = imageElement;
        this._url = url;
        if(type === undefined) {
            this._type = "png";
        }
    },
    
    getImage: function(xml, url, type) {
        if(url === undefined) {
            url = this._url;
        }
        if(type === undefined) {
            type = this._type;
        }

        dojo.rawXhrPost({
            url: this._url,

             postData : xml,
             headers: {"Content-Type": "text/xml"},
             handleAs: "text",

            preventCache: true,
            load : dojo.hitch(this, function(response, ioArgs) {
                var img = this._imageElement;  
                img.src = "data:image/" + type + ";charset=ASCII-US;base64," + response;
                return response;
            }),
            error: dojo.hitch(this, function(response, ioArgs) {
                return response;
            })
            
        });
    }
});

