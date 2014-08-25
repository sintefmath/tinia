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

dojo.provide("gui.ProxyModelCoverageReplaceFarthestAway");

dojo.declare("gui.ProxyModelCoverageReplaceFarthestAway", null, {


    constructor: function(glContext, ringSize, angleThreshold, zoomThreshold) {
        console.log("ProxyModelCoverageReplaceFarthestAway constructor: " + ringSize);
        this._gl = glContext;
        this._depthRingCursor = 0;
        if (ringSize>0) {
            this.proxyModelRing = new Array(ringSize);
            for (var i=0; i<ringSize; i++) {
                this.proxyModelRing[i] = new gui.ProxyModel(this._gl);
            }
        }
        this.mostRecentModel = new gui.ProxyModel(this._gl);
        this.bufferRingSize = ringSize;
        this._proxyModelReplacementAngle = angleThreshold;
        this._proxyModelReplacementZoom = zoomThreshold;
        console.log("ProxyModelCoverageReplaceFarthestAway constructor ended");
    },


    // If there is an unused slot, add the new proxy model.
    // If not, replace the one with (direction, scaling) farthest away from the new one, given that the (angle, zoom) requirement so dictates.
    _processDepthDataReplaceFarthestAway: function(model) {
        // console.log("processDepthDataReplaceFarthestAway: Considering adding new proxy model");
        if ( model.state != 2 ) {
            alert("processDepthDataReplaceFarthestAway: Incomplete proxy model - cannot process this!");
        }

        var addModel = false;

        // For this strategy, the cursor is not really important, but we can use it to detect whether or not we have filled the ring so far
        this._depthRingCursor = (this._depthRingCursor + 1) % this.bufferRingSize;
        if ( this.proxyModelRing[this._depthRingCursor].state == 0 ) {
            // The ring has not been filled, no need to find a model to throw out, we simply add the new one to the set
            // console.log("processDepthDataReplaceFarthestAway: ring not full, inserting directly");
            addModel = true;
        } else {
            // console.log("processDepthDataReplaceOldestWhenDifferent: dir                  = " + model.dir[0] + " " + model.dir[1] + " " + model.dir[2]);

            // We will now loop through the ring and find the one to throw out
            var worstIndx  = -1;
            var worstCosAngle = 10.0;
            var worstZoom  = 0.0;
            for (var i=0; i<this.bufferRingSize; i++) {
                // An assertion that should be removed when not debugging
                if ( this.proxyModelRing[i].state != 2 ) {
                    alert("processDepthDataReplaceOldestWhenDifferent: Incomplete proxy model added to ring.");
                }
                var cosAngle = vec3.dot( this.proxyModelRing[i].dir, model.dir );
                // console.log("processDepthDataReplaceFarthestAway: angle[" + i + "] = " + Math.acos(Math.min(1.0, cosAngle))/3.1415926535*180.0 + " (cosAngle = " + cosAngle + ")");
                var zoom = this.proxyModelRing[i].dist / model.dist;
                // console.log("processDepthDataReplaceFarthestAway: zoom factor[" + i + "] = " + zoom);
                if ( (cosAngle<worstCosAngle) || (zoom>worstZoom) ) {
                    worstCosAngle = Math.min( worstCosAngle, cosAngle );
                    worstZoom = Math.max( worstZoom, zoom );
                    worstIndx = i;
                }
            }
            // Should we add this new model to the ring, or forget about it?
            if ( (worstCosAngle<Math.cos(this._proxyModelReplacementAngle)) || (worstZoom>this._proxyModelReplacementZoom) ) {
                addModel = true;
                this._depthRingCursor = worstIndx;
            }
        }

        if (addModel) {
            this.proxyModelRing[this._depthRingCursor] = model;
            console.log("processDepthDataReplaceFarthestAway: inserted into slot " + this._depthRingCursor);
        } else {
            // console.log("processDepthDataReplaceFarthestAway: not inserting model");
        }

    },


    // Will use the replacement algorithm specified at construction time
    processDepthData: function(model) {
        this._processDepthDataReplaceFarthestAway(model);
    }


});
