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

//dojo.require("three.js");

dojo.provide("gui.ProxyModel");

dojo.declare("gui.ProxyModelCoverageAngles", null, {


    constructor: function(glContext, ringSize) {
        this.gl = glContext;
        this._direction = new Array(ringSize);
        this._dirSet    = new Array(ringSize);
        for (i=0; i<ringSize; i++) {
            this._direction[i] = vec3.create([0,0,0]);
            this._dirSet[i] = false;
        }
        console.log("ProxyModelCoverageAngles constructor ended");
    },


    findDirection: function(mv, pm) {
//        var Mat4 mv_inv = inverse( mv );
//        return -mv_inv[2];
    },


    addNewDirection: function(mv, pm, indx) {
        this._direction[i] = findDirection(mv, pm);
        this._dirSet[i]    = true;
    },


    newDirectionDistToCollection: function(mv, pm, splats) {
        var newBinsFilled = 0;
        for (i=0; i<splats.length; i++) {
            var v = pm * mv * splats[i];
            var bin_i = 0; // plane
            var bin_j = 0; // row
            var bin_k = 0; // column
            if ( this._coverage[ (bin_i * this._gridSize + bin_j) * this._gridSize + bin_k ] > 0 ) {
                newBinsFilled++;
            }
        }
        return newBinsFilled;
    }


});
