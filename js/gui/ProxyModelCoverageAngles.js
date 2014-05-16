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


    constructor: function(glContext, ringSize, angleThreshold, zoomThreshold) {
        console.log("ProxyModelCoverageAngles constructor: " + ringSize);
        this._gl = glContext;
        this._depthRingCursor = 0;
        this.proxyModelRing = new Array(ringSize);
        for (var i=0; i<ringSize; i++) {
            this.proxyModelRing[i] = new gui.ProxyModel(this._gl);
        }
        this._ringSize = ringSize;
        this._proxyModelReplacementAngle = angleThreshold;
        this._proxyModelReplacementZoom = zoomThreshold;
        console.log("ProxyModelCoverageAngles constructor ended");
    },


    // It is optionally added to the (ring) buffer, but always added to the "most recent model"
    _addModel: function(model) {
        this.proxyModelRing[this._depthRingCursor] = model;
        this._mostRecentModel = model;
    },


    processDepthDataReplaceOldest: function(model) {
        // console.log("processDepthDataReplaceOldest: Considering adding new proxy model");
        if ( model.state != 2 ) {
            alert("processDepthDataReplaceOldest: Incomplete proxy model - cannot process this!");
        }

        // Simply replacing the oldest proxy model with the new one.
        this._depthRingCursor = (this._depthRingCursor + 1) % this._ringSize;
        this._addModel(model);
        // console.log("processDepthDataReplaceOldest: inserted into slot " + this._depthRingCursor);
    },


    processDepthDataReplaceOldestWhenDifferent: function(model) {
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
            this._depthRingCursor = (this._depthRingCursor + 1) % this._ringSize;
            this._addModel(model);
            console.log("processDepthDataReplaceOldestWhenDifferent: inserted into slot " + this._depthRingCursor);
        } else {
            // console.log("processDepthDataReplaceOldestWhenDifferent: not inserting model");
        }
    },


    // If there is an unused slot, add the new proxy model.
    // If not, replace the one with (direction, scaling) farthest away from the new one, given that the (angle, zoom) requirement so dictates.

    processDepthDataReplaceFarthestAway: function(model) {
        // console.log("processDepthDataReplaceFarthestAway: Considering adding new proxy model");
        if ( model.state != 2 ) {
            alert("processDepthDataReplaceFarthestAway: Incomplete proxy model - cannot process this!");
        }

        var addModel = false;

        // For this strategy, the cursor is not really important, but we can use it to detect whether or not we have filled the ring so far
        this._depthRingCursor = (this._depthRingCursor + 1) % this._ringSize;
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
            for (var i=0; i<this._ringSize; i++) {
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
            this._addModel(model);
            console.log("processDepthDataReplaceFarthestAway: inserted into slot " + this._depthRingCursor);
        } else {
            // console.log("processDepthDataReplaceFarthestAway: not inserting model");
        }

    },


    // Calculating the coverage norm for the whole ring.
    _coverageNorm: function() {
        var totalCosAngleSum = 0.0;
        for (var i=0; i<this._ringSize; i++) {
            if ( this.proxyModelRing[i].state == 2 ) {
                for (var j=0; j<this._ringSize; j++) {
                    if ( (i!=j) && (this.proxyModelRing[j].state==2) ) {
                        var tmp = vec3.dot( this.proxyModelRing[i].dir, this.proxyModelRing[j].dir );
                        totalCosAngleSum += (tmp+1.0)*(tmp+1.0);
                    }
                }
            }
        }
        return totalCosAngleSum;
    },


    // If there is an unused slot, add the new proxy model.
    // If not, replace the one that maximizes the new sum of squared angles between all models.

    // One drawback with this strategy is that it may take very long before old models are replaced, unless the appropriate flag is set

    processDepthDataOptimizeCoverage: function(model) {
        console.clear();
        // An assertion that should be removed when not debugging
        if ( model.state != 2 ) {
            alert("processDepthDataOptimizeCoverage: Incomplete proxy model - cannot process this!");
        }

        var addModel = false;

        this._depthRingCursor = (this._depthRingCursor + 1) % this._ringSize;
        if ( this.proxyModelRing[this._depthRingCursor].state == 0 ) {
            // The ring has not been filled, no need to find a model to throw out, we simply add the new one to the set
            console.log("processDepthDataOptimizeCoverage: ring not full, inserting directly into slot " + this._depthRingCursor);
            addModel = true;
        } else {
            // console.log("processDepthDataOptimizeCoverage: dir                  = " + model.dir[0] + " " + model.dir[1] + " " + model.dir[2]);

            // Checking if we can improve the buffer by replacing an older model
            var minNorm = 1.0 + 4.0*this._ringSize * this._ringSize; // Should be larger than any possible sum. This makes sure some 'best_i' will always be found.
            var best_i = -1;
            for (var i=0; i<this._ringSize; i++) {
                var temporaryReplacedModel = this.proxyModelRing[i];
                this.proxyModelRing[i] = model;
                var norm = this._coverageNorm();
                this.proxyModelRing[i] = temporaryReplacedModel;
                console.log("  processDepthDataOptimizeCoverage: norm[" + i + "]        = " + norm);
                if ( norm < minNorm ) {
                    best_i = i;
                    minNorm = norm;
                    addModel = true;
                }
            }

            var oldNorm = this._coverageNorm();
            console.log("processDepthDataOptimizeCoverage: old coverage = " + oldNorm + ", new coverage = " + minNorm);

            if ( minNorm > 0.999*oldNorm ) {
                if ( minNorm < oldNorm ) {
                    console.log("  Improvement, but not by more than 10%, so we do not add the model after all.");
                }
                addModel = false;
            } else {
                console.log("  Improvement by more than 10%, replacing old model");
            }

            if ( !addModel ) { // If we are not to change model after considering all angles, we check if we are to override the decision due to zooming
                var zoom = this.proxyModelRing[best_i].dist / model.dist;
                console.log("Not adding model for rotation. For zooming? old_dist: " + this.proxyModelRing[best_i].dist + ", new dist: " + model.dist + ", zoom = " + zoom);
                if ( zoom > this._proxyModelReplacementZoom ) {
                    console.log("  Yes, adding due to zooming")
                    addModel = true;
                } else {
                    console.log("  No, not, adding");
                }
            }

            if (addModel) {
                this._depthRingCursor = best_i;
            }
        }

        if (addModel) {
            this._addModel(model);
            console.log("processDepthDataOptimizeCoverage: inserted into slot " + this._depthRingCursor);
        } else {
            console.log("processDepthDataOptimizeCoverage: not inserting model");
        }

    }


});
