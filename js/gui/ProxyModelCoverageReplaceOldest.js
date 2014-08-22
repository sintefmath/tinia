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

dojo.provide("gui.ProxyModelCoverageReplaceOldest");

dojo.declare("gui.ProxyModelCoverageReplaceOldest", null, {


    constructor: function(glContext, ringSize, angleThreshold, zoomThreshold) {
        console.log("ProxyModelCoverageReplaceOldest constructor: " + ringSize);
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
        console.log("ProxyModelCoverageReplaceOldest constructor ended");
    },


    _processDepthDataReplaceOldest: function(model) {
        // console.log("processDepthDataReplaceOldest: Considering adding new proxy model");
        if ( model.state != 2 ) {
            alert("processDepthDataReplaceOldest: Incomplete proxy model - cannot process this!");
        }

        // Simply replacing the oldest proxy model with the new one.
        this._depthRingCursor = (this._depthRingCursor + 1) % this.bufferRingSize;
        this.proxyModelRing[this._depthRingCursor] = model;
        // console.log("processDepthDataReplaceOldest: inserted into slot " + this._depthRingCursor);
    },


    // Will use the replacement algorithm specified at construction time
    processDepthData: function(model) {
        this._processDepthDataReplaceOldest(model);
    }


});
