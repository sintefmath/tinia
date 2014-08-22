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
        console.log("ProxyModelCoverageAngles constructor ended");
    },


    // Calculating the coverage norm for the whole ring.
    _coverageNorm: function() {
        // var t0 = Date.now();
        var totalCosAngleSum = 0.0;
        for (var i=0; i<this.bufferRingSize; i++) {
            if ( this.proxyModelRing[i].state == 2 ) {
                for (var j=0; j<this.bufferRingSize; j++) {
                    if ( (i!=j) && (this.proxyModelRing[j].state==2) ) {
                        var tmp = vec3.dot( this.proxyModelRing[i].dir, this.proxyModelRing[j].dir );
                        totalCosAngleSum += (tmp+1.0)*(tmp+1.0);
                    }
                }
            }
        }
        // console.log("_coverageNorm time: " + (Date.now()-t0));
        return totalCosAngleSum;
    },


    // If there is an unused slot, add the new proxy model.
    // If not, replace the one that maximizes the new sum of squared angles between all models.

    // One drawback with this strategy is that it may take very long before old models are replaced, unless the appropriate flag is set

    _processDepthDataOptimizeCoverage: function(model) {
        // var t0 = Date.now();
        // An assertion that should be removed when not debugging
        if ( model.state != 2 ) {
            alert("processDepthDataOptimizeCoverage: Incomplete proxy model - cannot process this!");
        }
        this.mostRecentModel = model;

        if (this.bufferRingSize>0) {
            var addModel = false;

            this._depthRingCursor = (this._depthRingCursor + 1) % this.bufferRingSize;
            if ( this.proxyModelRing[this._depthRingCursor].state == 0 ) {
                // The ring has not been filled, no need to find a model to throw out, we simply add the new one to the set
                console.log("processDepthDataOptimizeCoverage: ring not full, inserting directly into slot " + this._depthRingCursor);
                addModel = true;
            } else {
                // console.log("processDepthDataOptimizeCoverage: dir                  = " + model.dir[0] + " " + model.dir[1] + " " + model.dir[2]);

                // Can we improve the coverage with respect to angles?
                var minNorm = 1e99;
                var best_i = -1;
                for (var i=0; i<this.bufferRingSize; i++) {
                    var temporaryReplacedModel = this.proxyModelRing[i];
                    this.proxyModelRing[i] = model;
                    var norm = this._coverageNorm();
                    this.proxyModelRing[i] = temporaryReplacedModel;
                    // console.log("  processDepthDataOptimizeCoverage: norm[" + i + "]        = " + norm);
                    if ( norm < minNorm ) {
                        best_i = i;
                        minNorm = norm;
                    }
                }
                var oldNorm = this._coverageNorm();
                // console.log("processDepthDataOptimizeCoverage: old coverage = " + oldNorm + ", new coverage = " + minNorm);
                if ( minNorm > 0.9*oldNorm ) {
                    if ( minNorm < oldNorm ) {
                        // console.log("  Improvement, but not by more than 10%, so we do not add the model after all.");
                    } else {
                        // console.log("  No improvement, we do not add the model.");
                    }
                    best_i = -1;
                } else {
                    // console.log("  Improvement by more than 10%, replacing old model");
                }

                // Add due to zooming? We let "angle-optimization" override "zoom-optimization".
                if ( best_i < 0 ) {
                    // Checking if we can improve the buffer by replacing an older model
                    var minNorm = 0.0;
                    for (var i=0; i<this.bufferRingSize; i++) {
                        var zoom = this.proxyModelRing[i].dist / model.dist;
                        // console.log("  processDepthDataOptimizeCoverage: zoom[" + i + "]        = " + zoom);
                        if ( zoom > minNorm ) {
                            best_i = i;
                            minNorm = zoom;
                        }
                    }
                    // console.log("Not adding model for rotation. For zooming? old_dist: " + this.proxyModelRing[best_i].dist + ", new dist: " + model.dist + ", zoom = " + zoom);
                    if ( zoom > this._proxyModelReplacementZoom ) {
                        // console.log("  Yes, adding due to zooming")
                    } else {
                        // console.log("  No, not, adding, zoom not large enough");
                        best_i = -1;
                    }
                }
                if ( best_i >= 0 ) {
                    this._depthRingCursor = best_i;
                    addModel = true;
                }

            }

            if (addModel) {
                this.proxyModelRing[this._depthRingCursor] = model;
                // console.log("processDepthDataOptimizeCoverage: inserted into slot " + this._depthRingCursor);
            } else {
                // console.log("processDepthDataOptimizeCoverage: not inserting model");
            }

        } // end of if (bufferRingSize>0)
        // console.log("processDepthDataOptimizeCoverage time: " + (Date.now()-t0));
    },


    // Will use the replacement algorithm specified at construction time
    processDepthData: function(model) {
        this._processDepthDataOptimizeCoverage(model);
    }


});
