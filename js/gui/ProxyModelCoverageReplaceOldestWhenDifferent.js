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

dojo.provide("gui.ProxyModelCoverageReplaceOldestWhenDifferent");

dojo.declare("gui.ProxyModelCoverageReplaceOldestWhenDifferent", null, {


    constructor: function(glContext, ringSize, angleThreshold, zoomThreshold) {
        console.log("ProxyModelCoverageReplaceOldestWhenDifferent constructor: " + ringSize);
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
        console.log("ProxyModelCoverageReplaceOldestWhenDifferent constructor ended");
    },


    _processDepthDataReplaceOldestWhenDifferent: function(model) {
        // console.log("processDepthDataReplaceOldestWhenDifferent: Considering adding new proxy model");
        if ( model.state != 2 ) {
            alert("processDepthDataReplaceOldestWhenDifferent: Incomplete proxy model - cannot process this!");
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
                alert("processDepthDataReplaceOldestWhenDifferent: Incomplete proxy model added to ring.");
            }
            // console.log("processDepthDataReplaceOldestWhenDifferent: There is a recently added model in slot " + this.__depthRingCursor + ", comparing directions...");
            var mostRecentlyAddedDir = this.proxyModelRing[this._depthRingCursor].dir;
            // console.log("processDepthDataReplaceOldestWhenDifferent: mostRecentlyAddedDir = " + mostRecentlyAddedDir[0] + " " + mostRecentlyAddedDir[1] + " " + mostRecentlyAddedDir[2]);
            // console.log("processDepthDataReplaceOldestWhenDifferent: dir                  = " + model.dir[0] + " " + model.dir[1] + " " + model.dir[2]);
            var cosAngle = vec3.dot( mostRecentlyAddedDir, model.dir );
            console.log("processDepthDataReplaceOldestWhenDifferent: angle = " + Math.acos(Math.min(1.0, cosAngle))/3.1415926535*180.0 + " (cosAngle = " + cosAngle + ")");
            var zoom = this.proxyModelRing[this._depthRingCursor].dist / model.dist;
            console.log("processDepthDataReplaceOldestWhenDifferent: zoom factor = " + zoom);
            if ( (cosAngle<Math.cos(this._proxyModelReplacementAngle)) || (zoom>this._proxyModelReplacementZoom) ) {
                addModel = true;
            }
        }
        if (addModel) {
            this._depthRingCursor = (this._depthRingCursor + 1) % this.bufferRingSize;
            this.proxyModelRing[this._depthRingCursor] = model;
            console.log("processDepthDataReplaceOldestWhenDifferent: inserted into slot " + this._depthRingCursor);
        } else {
            // console.log("processDepthDataReplaceOldestWhenDifferent: not inserting model");
        }
    },


    // Will use the replacement algorithm specified at construction time
    processDepthData: function(model) {
        this._processDepthDataReplaceOldestWhenDifferent(model);
    }


});
