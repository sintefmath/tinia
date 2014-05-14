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

dojo.require("gui.ProxyModel");

dojo.provide("gui.ProxyModelCoverageAngles");

dojo.declare("gui.ProxyModelCoverageAngles", null, {


    constructor: function(glContext, ringSize) {
        console.log("ProxyModelCoverageAngles constructor: " + ringSize);

        this._proxyModelReplacementAngle = (180.0/ringSize) / 180.0*3.1415926535;
        this._proxyModelReplacementZoom = 1.2;

        // ---------------- End of configuration section -----------------

        this._gl = glContext;
        this._depthRingCursor = 0;
        this.proxyModelRing = new Array(ringSize);
        for (i=0; i<ringSize; i++) {
            this.proxyModelRing[i] = new gui.ProxyModel(this._gl);
        }

        this._ringSize = ringSize;
        console.log("ProxyModelCoverageAngles constructor ended");
    },


//    // If there is a free slot, we return that.
//    // If not, the slot for the proxy model farthest away is returned.
//    // It is assumed that the caller will make use of the slot, i.e., it will be flagged as in use after this call.
//    bestSlot: function(model) {
//        console.log("bestSlot: nextFreeSlot=" + this._nextFreeSlot + " ringSize=" + this._ringSize);
//        if (this._nextFreeSlot<this._ringSize) {
//            this._nextFreeSlot++;
//            console.log("bestSlot: returning slot " + (this._nextFreeSlot-1));
//            return this._nextFreeSlot-1;
//        } else {
//            var dir   = vec3.create( [-model.to_world[8], -model.to_world[9], -model.to_world[10]] );
//            var bestAngle = vec3.dot( dir, this._direction[0] );
//            console.log("bestSlot: first angle: " + bestAngle);
//            var bestI = 0;
//            for (i=1; i<this._ringSize; i++) {
//                var cosAngle = vec3.dot( dir, this._direction[i] );
//                console.log("bestSlot: angle for i=" + i + ": " + cosAngle);
//                if (cosAngle<bestAngle) {
//                    bestAngle = cosAngle;
//                    bestI = i;
//                }
//            }
//            return bestI;
//        }
//    },


//    update: function(model, slot) {
//        var d = vec3.create( [-model.to_world[8], -model.to_world[9], -model.to_world[10]] );
//        console.log("update: Setting dir for slot i=" + slot + " to: " + d[0] + " " + d[1] + " " + d[2]);
//        this._direction[slot] = vec3.create( [-model.to_world[8], -model.to_world[9], -model.to_world[10]] );
//    },



    processDepthDataReplaceOldest: function(model) {
        // console.log("processDepthDataReplaceOldest: Considering adding new proxy model");
        if ( model.state != 2 ) {
            throw "processDepthDataReplaceOldest: Incomplete proxy model - cannot process this!";
        }

        // Simply replacing the oldest proxy model with the new one.
        this._depthRingCursor = (this._depthRingCursor + 1) % this._ringSize;
        this.proxyModelRing[this._depthRingCursor] = model;
        // console.log("processDepthDataReplaceOldest: inserted into slot " + this._depthRingCursor);
    },


    processDepthDataReplaceOldestWhenDifferent: function(model) {
        // console.log("processDepthDataReplaceOldestWhenDifferent: Considering adding new proxy model");
        if ( model.state != 2 ) {
            throw "processDepthDataReplaceOldestWhenDifferent: Incomplete proxy model - cannot process this!";
        }

        // Skip if the new one is close to the most recently added. Otherwise, go for strategy processDepthDataReplaceOldest.
        var addModel = false;
        if ( this.proxyModelRing[this._depthRingCursor].state == 0 ) {
            // There is not a "most recently added model", i.e., this is the first to be added, so we add it unconditionally
            // console.log("processDepthDataReplaceOldestWhenDifferent: No recently added model, adding one. cursor=" + this.__depthRingCursor);
            addModel = true;
        } else {
            // There is a "most recently added model", i.e., we can compare directions
            if ( this.proxyModelRing[this._depthRingCursor].state != 2 ) {
                throw "processDepthDataReplaceOldestWhenDifferent: Incomplete proxy model added to ring.";
            }
            // console.log("processDepthDataReplaceOldestWhenDifferent: There is a recently added model in slot " + this.__depthRingCursor + ", comparing directions...");
            var mostRecentlyAddedDir = vec3.create( [-this.proxyModelRing[this._depthRingCursor].to_world[8],
                                                     -this.proxyModelRing[this._depthRingCursor].to_world[9],
                                                     -this.proxyModelRing[this._depthRingCursor].to_world[10]] );
            vec3.normalize(mostRecentlyAddedDir);
            // console.log("processDepthDataReplaceOldestWhenDifferent: mostRecentlyAddedDir = " + mostRecentlyAddedDir[0] + " " + mostRecentlyAddedDir[1] + " " + mostRecentlyAddedDir[2]);
            var dir = vec3.create( [-model.to_world[8],
                                    -model.to_world[9],
                                    -model.to_world[10]] );
            vec3.normalize(dir);
            // console.log("processDepthDataReplaceOldestWhenDifferent: dir                  = " + dir[0] + " " + dir[1] + " " + dir[2]);
            var cosAngle = vec3.dot( mostRecentlyAddedDir, dir );
            console.log("processDepthDataReplaceOldestWhenDifferent: angle = " + Math.acos(Math.min(1.0, cosAngle))/3.1415926535*180.0 + " (cosAngle = " + cosAngle + ")");
            var mostRecentlyAddedDist = vec3.create( [-this.proxyModelRing[this._depthRingCursor].to_world[12],
                                                      -this.proxyModelRing[this._depthRingCursor].to_world[13],
                                                      -this.proxyModelRing[this._depthRingCursor].to_world[14]] );
            var dist = vec3.create( [-model.to_world[12],
                                     -model.to_world[13],
                                     -model.to_world[14]] );
            var zoom = vec3.length(mostRecentlyAddedDist) / vec3.length(dist);
            console.log("processDepthDataReplaceOldestWhenDifferent: zoom factor = " + zoom);
            if ( (cosAngle<Math.cos(this._proxyModelReplacementAngle)) || (zoom>this._proxyModelReplacementZoom) ) {
                addModel = true;
            }
        }
        if (addModel) {
            this._depthRingCursor = (this._depthRingCursor + 1) % this._ringSize;
            this.proxyModelRing[this._depthRingCursor] = model;
            console.log("processDepthDataReplaceOldestWhenDifferent: inserted into slot " + this._depthRingCursor);
        } else {
            // console.log("processDepthDataReplaceOldestWhenDifferent: not inserting model");
        }
    },


    processDepthDataReplaceFarthestAway: function(model) {
        // console.log("processDepthDataReplaceFarthestAway: Considering adding new proxy model");
        if ( model.state != 2 ) {
            throw "processDepthDataReplaceFarthestAway: Incomplete proxy model - cannot process this!";
        }

        // if there is an unused slot, add the new proxy model.
        // If not, replace the one with (direction, scaling) farthest away from the new one.

        var addModel = false;

        // For this strategy, the cursor is not really important, but we can use it to detect whether or not we have filled the ring so far
        this._depthRingCursor = (this._depthRingCursor + 1) % this._ringSize;
        if ( this.proxyModelRing[this._depthRingCursor].state == 0 ) {
            // The ring has not been filled, no need to find a model to throw out, we simply add the new one to the set
            // console.log("processDepthDataReplaceFarthestAway: ring not full, inserting directly");
            addModel = true;
        } else {
            var dir = vec3.create( [-model.to_world[8],
                                    -model.to_world[9],
                                    -model.to_world[10]] );
            vec3.normalize(dir);
            // console.log("processDepthDataReplaceOldestWhenDifferent: dir                  = " + dir[0] + " " + dir[1] + " " + dir[2]);
            var dist = vec3.create( [-model.to_world[12],
                                     -model.to_world[13],
                                     -model.to_world[14]] );
            var zoom = vec3.length(mostRecentlyAddedDist) / vec3.length(dist);
            console.log("processDepthDataReplaceOldestWhenDifferent: zoom factor = " + zoom);

            // We will now loop through the ring and find the one to throw out
            var worst_indx = 0;
            var worst_angle = 0.0;
            var worst_zoom = 0.0;
            for (i=0; i<this._ringSize; i++) {
                // An assertion that should be removed when not debugging
                if ( this.proxyModelRing[this._depthRingCursor].state != 2 ) {
                    throw "processDepthDataReplaceOldestWhenDifferent: Incomplete proxy model added to ring.";
                }

//                // console.log("processDepthDataReplaceOldestWhenDifferent: There is a recently added model in slot " + this.__depthRingCursor + ", comparing directions...");
//                var mostRecentlyAddedDir = vec3.create( [-this.proxyModelRing[this._depthRingCursor].to_world[8],
//                                                         -this.proxyModelRing[this._depthRingCursor].to_world[9],
//                                                         -this.proxyModelRing[this._depthRingCursor].to_world[10]] );
//                vec3.normalize(mostRecentlyAddedDir);
//                // console.log("processDepthDataReplaceOldestWhenDifferent: mostRecentlyAddedDir = " + mostRecentlyAddedDir[0] + " " + mostRecentlyAddedDir[1] + " " + mostRecentlyAddedDir[2]);
//                var cosAngle = vec3.dot( mostRecentlyAddedDir, dir );
//                console.log("processDepthDataReplaceOldestWhenDifferent: angle = " + Math.acos(Math.min(1.0, cosAngle))/3.1415926535*180.0 + " (cosAngle = " + cosAngle + ")");
//                var mostRecentlyAddedDist = vec3.create( [-this.proxyModelRing[this._depthRingCursor].to_world[12],
//                                                          -this.proxyModelRing[this._depthRingCursor].to_world[13],
//                                                          -this.proxyModelRing[this._depthRingCursor].to_world[14]] );
//                if ( (cosAngle<Math.cos(this._proxyModelReplacementAngle)) || (zoom>this._proxyModelReplacementZoom) ) {
//                    addModel = true;
//                }
            }

            // we have what we need. only replace if sufficiently bad
            // set addModel if we are to add, and also set ringCurs to proper value

        }

        // Same as before. No! Should not set to the ring Cursor + 1. Should set to ringCursor, so the value gets correct for later usage.
        if (addModel) {
            this._depthRingCursor = (this._depthRingCursor + 1) % this._ringSize;
            this.proxyModelRing[this._depthRingCursor] = model;
            console.log("processDepthDataReplaceOldestWhenDifferent: inserted into slot " + this._depthRingCursor);
        } else {
            // console.log("processDepthDataReplaceOldestWhenDifferent: not inserting model");
        }

    },


});
