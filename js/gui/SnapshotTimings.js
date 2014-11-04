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
        // Currently not in use:
//        this._currentSnapType = "";
//        this.currentSnapTypeCntr = 0;
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

        // All different snapshot types mixed together:
        if (this._listCursors.all) {
            // This type already exists
        } else {
            this._listCursors.all = 0;
        }
        this._timingList[this._listCursors.all].all = time;
        this._listCursors.all = ( this._listCursors.all + 1 ) % this._n;

        // Keeping track of changing types
//        if ( snapType == this._currentSnapType ) {
//            this.currentSnapTypeCntr++;
//        } else {
//            this.currentSnapTypeCntr = 1;
//            this._currentSnapType = snapType;
//        }

        if ( snapType == "ap" ) {
            if ( this._remainingFramesForAPrecording ) {
                if ( this._remainingFramesForAPrecording == 1 ) {
                    this._recordedAPtime = this.getAvgTime( snapType ); // We store this time for usage by the auto-select stuff in Canvas.js
                }
                if ( this._remainingFramesForAPrecording > 0 ) {
                    this._remainingFramesForAPrecording--;
                }
            }
        }
    },


    print: function() {
        var console_string = "##### timing averages: ";
        for ( var snapType in this._listCursors ) {
            console_string = console_string + snapType + ": " + this.getAvgTime(snapType) + "  ";
        }
        // console_string = console_string + " (ap/png=" + (this.getAvgTime("ap")/this.getAvgTime("png")) + ")";
        console.log(console_string);
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


    // Returning 0 if we don't have any results.
    getLatestTime: function(snapType) {
        if (this._listCursors[snapType]) {
            // This type exists
            return this._timingList[ ( this._listCursors[snapType] - 1 + this._n ) % this._n ][ snapType ];
        } else {
            return 0;
        }
    },


    // Returning the key for the highest time lower than the target time, excluding those with avg time zero.
    // If there are no times lower than the target, returning the over all lowest time.
    getFastest: function(targetTime) {
        var bestType = "";
        var lowestType = "";
        var best = 0;
        var lowest = 999999;
        for ( var snaptype in this._listCursors ) {
            var t = this.getAvgTime(snaptype);
            if (t!=0) {
                if ( (t>best) && (t<targetTime) ) {
                    best = t;
                    bestType = snaptype;
                    // console.log("  new best: " + bestType + " (" + best + ")");
                }
                if ( t < lowest ) {
                    lowest = t;
                    lowestType = snaptype;
                    // console.log("  new lowest: " + lowestType + " (" + lowest + ")");
                }
            }
        }
        if (best>0) {
            return bestType;
        } else {
            return lowestType;
        }
    },


    initiateAPrecording: function() {
//        console.log("initiateAPrecording starting ********************************");
        this._remainingFramesForAPrecording = this._n;
//        console.log("initiateAPrecording returning");
    },


    // Returning 0 if no average yet obtained
    getRecordedAPtime: function() {
        // console.log("getRecordedAPtime starting");
        // if ( !(this._remainingFramesForAPrecording) ) {
        if ( typeof(this._remainingFramesForAPrecording) === 'undefined' ) {
//            console.log(".......... Strange! Requesting APtime before initiateAPrecording has been called?!?!");
//            console.log( this );
            return 0;
        } else {
            if ( this._remainingFramesForAPrecording == 0 ) {
//                console.log(".......... Returning time " + this._recordedAPtime);
                return this._recordedAPtime;
            } else {
//                console.log(".......... _remainingFramesForAPrecording = " + _remainingFramesForAPrecording + ", returning zero for time");
                return 0;
            }
        }
    }


});
