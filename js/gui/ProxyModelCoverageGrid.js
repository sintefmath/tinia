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

dojo.provide("gui.ProxyModel");

dojo.declare("gui.ProxyModelCoverageGrid", null, {


    constructor: function(glContext, gridSize) {
        this.gl = glContext;
        this._coverage = new Array(gridSize*gridSize*gridSize);
        for (i=0; i<gridSize*gridSize*gridSize; i++)
            this._coverage[i] = 0;
        this._subGrid = null;
        console.log("ProxyModelCoverageGrid constructor ended");
    },


    // Binning the splat set
    // To be GPU-ified!
    addNewSplatSet: function(mv, pm, splats, addOrRemove) {
        for (i=0; i<splats.length; i++) {
            var v = pm * mv * splats[i];
            var bin_i = 0; // plane
            var bin_j = 0; // row
            var bin_k = 0; // column
            this._coverage[ (bin_i * this._gridSize + bin_j) * this._gridSize + bin_k ] += addOrRemove;
        }
    },


    // Testing coverage of new splat set
    // To be GPU-ified!
    testNewSplatSet: function(mv, pm, splats) {
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
