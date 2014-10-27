/* Copyright STIFTELSEN SINTEF 2014
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

dojo.provide("gui.SnapshotTimings");

//dojo.require("3rdparty.webgl-utils");
//dojo.require("renderlist.RenderList");
//dojo.require("renderlist.RenderListParser" );
//dojo.require("renderlist.RenderListRenderer" );
//dojo.require("dijit._Widget");
//dojo.require("gui.TrackBallViewer");
//dojo.require("dojo.touch");
//dojo.require("gui.ProxyRenderer");




dojo.declare("gui.SnapshotTimings", null, {


    constructor: function() {
//        console.log("timings constructor");
        this._timingList = Array(); // new Array() ??
        this._listCursors = {};
        this._n = 5;
        for (var i=0; i<this._n; i++) {
            this._timingList[i] = {};
        }

    },


    update: function(snapType, time) {
//        console.log("update: snapType=" + snapType + ", time=" + time);
        if (this._listCursors[snapType]) {
            // This type already exists
        } else {
            this._listCursors[snapType] = 0;
        }
        this._timingList[ this._listCursors[snapType] ][ snapType ] = time;
        this._listCursors[snapType] = ( this._listCursors[snapType] + 1 ) % this._n;
//        console.log("cursors:");
//        console.log( this._listCursors );
//        console.log("timings:");
//        for (var i=0; i<5; i++) {
//            console.log( this._timingList[i] );
//        }
    },


    print: function() {
//        console.log("##### cursors:");
//        console.log( this._listCursors );
//        console.log("##### timings:");
//        for (var i=0; i<this._n; i++) {
//            console.log( this._timingList[i] );
//        }
        console.log("##### timing averages:");
        for ( var snapType in this._listCursors ) {
            console.log( snapType + ": " + this.getAvgTime(snapType) );
        }
    },


    // Returning 0 if we don't have a full ring of results.
    getAvgTime: function(snapType) {
        // console.log("getAvgTime: snapType=" + snapType);
        var t = 0;
        var n = 0;
        for (var i=0; i<this._n; i++) {
            if (this._timingList[ i ][ snapType ]) {
                t += this._timingList[ i ][ snapType ];
                n++;
            }
        }
        if ( n < this._n ) {
            return 0;
        } else {
            // console.log( "returning t=" + t + " divided by n=" + n + ", =" + t/n );
            return t/n;
        }
    },


});
