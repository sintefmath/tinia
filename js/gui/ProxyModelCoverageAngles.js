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
        this._modelCosAngleSum = new Array(ringSize); // For each model[i]: Sum cos(model[i].dir, model[j].dir), i!=j
        for (i=0; i<ringSize; i++) {
            this.proxyModelRing[i] = new gui.ProxyModel(this._gl);
            this._modelCosAngleSum[i] = 0.0;
        }
        this._ringSize = ringSize;
        this._proxyModelReplacementAngle = angleThreshold;
        this._proxyModelReplacementZoom = zoomThreshold;
        this._cosAngleSum = 0.0;
        console.log("ProxyModelCoverageAngles constructor ended");
    },


    // It is optionally added to the (ring) buffer, but always added to the "most recent model"
    _addModel: function(model, addToRing) {
        if (addToRing) {
            var dir_new = vec3.create( [-model.to_world[8], -model.to_world[9], -model.to_world[10]] );
            vec3.normalize(dir_new);
            // If there is a model in the slot, updating the cosAngle variables is a bit more involved, since we must subtract the old contribution from all other slots
            var newModelCosAngleSum = 0.0;
            var oldTotal = this._cosAngleSum;
            if ( this.proxyModelRing[this._depthRingCursor].state == 2 ) {
                var dir_old = vec3.create( [-this.proxyModelRing[this._depthRingCursor].to_world[8],
                                            -this.proxyModelRing[this._depthRingCursor].to_world[9],
                                            -this.proxyModelRing[this._depthRingCursor].to_world[10]] );
                vec3.normalize(dir_old);
                this._cosAngleSum -= this._modelCosAngleSum[this._depthRingCursor];     // Subtracting from the total the old model's contribution
                this.proxyModelRing[this._depthRingCursor].state = 0;                   // Trick to avoid testing for i!=this._depthRingCursor in the loop below
                // Updating sums belonging to existing models, and the new one, in the same loop
                for (i=0; i<this._ringSize; i++) {
                    if ( this.proxyModelRing[i].state == 2 ) {
                        var dir_i = vec3.create( [-this.proxyModelRing[i].to_world[8],
                                                  -this.proxyModelRing[i].to_world[9],
                                                  -this.proxyModelRing[i].to_world[10]] );
                        vec3.normalize(dir_i);
                        this._modelCosAngleSum[i] -= vec3.dot( dir_i, dir_old );        // Removing old contribution
                        this._modelCosAngleSum[i] += vec3.dot( dir_i, dir_new );        // Adding new contribution
                        newModelCosAngleSum       += vec3.dot( dir_i, dir_new );        // Building sum for new model
                    } else {
//                        this._modelCosAngleSum[i] += 1.0;
//                        newModelCosAngleSum       += 1.0;
                    }
                }
                console.log("_addModel: replacement-branch, old total: " + oldTotal + ", new total: " + (this._cosAngleSum+newModelCosAngleSum));
            } else {
                // There is no old model to subtract contributions for
                // Updating sums belonging to existing models, and the new one, in the same loop
                for (i=0; i<this._ringSize; i++) {
                    if ( this.proxyModelRing[i].state == 2 ) {
                        var dir_i = vec3.create( [-this.proxyModelRing[i].to_world[8],
                                                  -this.proxyModelRing[i].to_world[9],
                                                  -this.proxyModelRing[i].to_world[10]] );
                        vec3.normalize(dir_i);
                        this._modelCosAngleSum[i] += vec3.dot( dir_i, dir_new );        // Adding new contribution
                        newModelCosAngleSum       += vec3.dot( dir_i, dir_new );        // Building sum for new model
                    } else {
//                        this._modelCosAngleSum[i] += 1.0;
//                        newModelCosAngleSum       += 1.0;
                    }
                }
                console.log("_addModel: new model branch, old total:   " + oldTotal + ", new total: " + (this._cosAngleSum+newModelCosAngleSum));
            }
            this._cosAngleSum += newModelCosAngleSum;                                   // Uppdating total with new contribution
            // Finally, inserting the model, and remembering the cosAngle entry also
            this.proxyModelRing[this._depthRingCursor]    = model;
            this._modelCosAngleSum[this._depthRingCursor] = newModelCosAngleSum;
        }
        this._mostRecentModel = model;
    },


    processDepthDataReplaceOldest: function(model) {
        // console.log("processDepthDataReplaceOldest: Considering adding new proxy model");
        if ( model.state != 2 ) {
            alert("processDepthDataReplaceOldest: Incomplete proxy model - cannot process this!");
        }

        // Simply replacing the oldest proxy model with the new one.
        this._depthRingCursor = (this._depthRingCursor + 1) % this._ringSize;
        this._addModel(model, true);
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
            this._addModel(model, true);
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
            var dir = vec3.create( [-model.to_world[8], -model.to_world[9], -model.to_world[10]] );
            vec3.normalize(dir);
            // console.log("processDepthDataReplaceOldestWhenDifferent: dir                  = " + dir[0] + " " + dir[1] + " " + dir[2]);
            var dist = vec3.create( [-model.to_world[12], -model.to_world[13], -model.to_world[14]] );

            // We will now loop through the ring and find the one to throw out
            var worstIndx  = -1;
            var worstCosAngle = 10.0;
            var worstZoom  = 0.0;
            for (i=0; i<this._ringSize; i++) {
                // An assertion that should be removed when not debugging
                if ( this.proxyModelRing[i].state != 2 ) {
                    alert("processDepthDataReplaceOldestWhenDifferent: Incomplete proxy model added to ring.");
                }

                var dir_i = vec3.create( [-this.proxyModelRing[i].to_world[8], -this.proxyModelRing[i].to_world[9], -this.proxyModelRing[i].to_world[10]] );
                vec3.normalize(dir_i);
                var dist_i = vec3.create( [-this.proxyModelRing[i].to_world[12], -this.proxyModelRing[i].to_world[13], -this.proxyModelRing[i].to_world[14]] );

                var cosAngle = vec3.dot( dir_i, dir );
                // console.log("processDepthDataReplaceFarthestAway: angle[" + i + "] = " + Math.acos(Math.min(1.0, cosAngle))/3.1415926535*180.0 + " (cosAngle = " + cosAngle + ")");
                var zoom = vec3.length(dist_i) / vec3.length(dist);
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
            this._addModel(model, true);
            console.log("processDepthDataReplaceFarthestAway: inserted into slot " + this._depthRingCursor);
        } else {
            // console.log("processDepthDataReplaceFarthestAway: not inserting model");
        }

    },


    // Going through all recorded models, and accumulating cosAngle against the specified, new, model.
    // Note that the computed value cannot replace a value in the ring, since the value to be used in the ring should not include the contribution from the new model against the removed ring member!
    _sumCosAngles: function(model) {
        var cosAngleSum = 0.0;
        var dir = vec3.create( [-model.to_world[8], -model.to_world[9], -model.to_world[10]] );
        vec3.normalize(dir);
        for (i=0; i<this._ringSize; i++) {
            if ( this.proxyModelRing[i].state == 2 ) {
                var dir_i = vec3.create( [-this.proxyModelRing[i].to_world[8], -this.proxyModelRing[i].to_world[9], -this.proxyModelRing[i].to_world[10]] );
                vec3.normalize(dir_i);
                var cosAngle = vec3.dot( dir_i, dir );
                cosAngleSum += cosAngle;
            }
        }
        return cosAngleSum;
    },


    // Calculating the coverage norm for the whole ring.
    _totalCosAngles: function() {
        var totalCosAngleSum = 0.0;
        for (i=0; i<this._ringSize; i++) {
            if ( this.proxyModelRing[i].state == 2 ) {
                var dir_i = vec3.create( [-this.proxyModelRing[i].to_world[8], -this.proxyModelRing[i].to_world[9], -this.proxyModelRing[i].to_world[10]] );
                vec3.normalize(dir_i);
                for (j=0; j<this._ringSize; j++) {
                    if ( (i!=j) && (this.proxyModelRing[j].state==2) ) {
                        var dir_j = vec3.create( [-this.proxyModelRing[j].to_world[8], -this.proxyModelRing[j].to_world[9], -this.proxyModelRing[j].to_world[10]] );
                        vec3.normalize(dir_j);
                        var cosAngle = vec3.dot( dir_i, dir_j );
                        totalCosAngleSum += cosAngle;
                    }
                }
            }
        }
        return totalCosAngleSum;
    },


                 // we need an update-method that does more or less what addModel does, of course we cannot use just the single loop below, that makes no sense.
                 // (For each model, it is not enough to adjust total + model-contribution, we must also adjust the model-contribution to each ring-model-contribution!)
                 // (No! Not for the total, which is all we need in this context...) (Yes! Because we must have a loop (i) for all candidate-slots, and then another loop (j)
                 // for all subtractions of model-j-against-model-i-contributions to remove...)
                 // Making a baseline version first, with a separate function. This will thus make an O(n^3) algo, but it should be easier to get right...)


    // If there is an unused slot, add the new proxy model.
    // If not, replace the one that maximizes the new sum of squared angles between all models.

    // One drawback with this strategy is that it may take very long before old models are replaced, unless the appropriate flag is set

    processDepthDataOptimizeCoverage: function(model) {
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
            var dir = vec3.create( [-model.to_world[8], -model.to_world[9], -model.to_world[10]] );
            vec3.normalize(dir);
            // console.log("processDepthDataOptimizeCoverage: dir                  = " + dir[0] + " " + dir[1] + " " + dir[2]);
            var newCosAngleSum = this._sumCosAngles(model);

            // Checking if we can improve the buffer by replacing an older model
            var minCosAngleSum = 1.0 + this._ringSize * this._ringSize; // Should be larger than any possible sum. This makes sure some 'best_i' will always be found.
            console.log("processDepthDataOptimizeCoverage: cosAngleSum candidate (including model to be replaced!) = " + newCosAngleSum + ", minCosAngleSum = " + minCosAngleSum );
            var best_i = -1;
            for (i=0; i<this._ringSize; i++) {
                // An assertion that should be removed when not debugging
                if ( this.proxyModelRing[i].state != 2 ) {
                    alert("processDepthDataOptimizeCoverage: Incomplete proxy model has somehow been added to ring.");
                }
                var dir_i = vec3.create( [-this.proxyModelRing[i].to_world[8], -this.proxyModelRing[i].to_world[9], -this.proxyModelRing[i].to_world[10]] );
                vec3.normalize(dir_i);
                var cosAngleNewAndCurrent = vec3.dot( dir, dir_i );
//                console.log("  processDepthDataOptimizeCoverage: angle[" + i + "]           = " +
//                            Math.acos(Math.min(1.0, cosAngleNewAndCurrent))/3.1415926535*180.0 + " (cosAngle = " + cosAngleNewAndCurrent + ")" );
                var newTotal = this._cosAngleSum - this._modelCosAngleSum[i] + newCosAngleSum - cosAngleNewAndCurrent;
                // console.log("  processDepthDataOptimizeCoverage: newTotal[" + i + "]        = " + newTotal + ", old value = " + this._modelCosAngleSum[i]);
                if ( newTotal < minCosAngleSum ) {
                    // console.log("    improvement");
                    best_i = i;
                    minCosAngleSum = newTotal;
                    addModel = true;
                }
            }

            console.log("processDepthDataOptimizeCoverage: old total = " + this._cosAngleSum + ", new value = " + minCosAngleSum);

            if ( minCosAngleSum > 0.9*this._cosAngleSum ) {
                if ( minCosAngleSum < this._cosAngleSum ) {
                    console.log("Improvement, but not by more than 10%, so we do not add the model after all.");
                }
                addModel = false;
            } else {
                console.log("Improvement by more than 10%, replacing old model! New: " + minCosAngleSum + ", old: " + this._cosAngleSum);
                // An assertion that should be removed when not debugging
                if ( !addModel ) {
                    alert("processDepthDataOptimizeCoverage: Huh?!");
                }
            }

            if ( !addModel ) { // If we are not to change model after considering all angles, we check if we are to override the decision due to zooming
                var dist = vec3.create( [-model.to_world[12], -model.to_world[13], -model.to_world[14]] );
                var dist_i = vec3.create( [-this.proxyModelRing[best_i].to_world[12], -this.proxyModelRing[best_i].to_world[13], -this.proxyModelRing[best_i].to_world[14]] );
                if ( vec3.length(dist_i)/vec3.length(dist) > this._proxyModelReplacementZoom ) {
                    console.log("Not adding model for angle, but for zooming")
                    addModel = true;
                }
            }

            if (addModel) {
                this._depthRingCursor = best_i;
            }
        }

        if (addModel) {
            this._addModel(model, true, "hei");
            console.log("processDepthDataOptimizeCoverage: inserted into slot " + this._depthRingCursor);
        } else {
            // console.log("processDepthDataOptimizeCoverage: not inserting model");
        }

    }


});
