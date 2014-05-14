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


    constructor: function(ringSize) {
        console.log("ProxyModelCoverageAngles constructor: " + ringSize);
        this._direction = new Array(ringSize);
        this._ringSize = ringSize;
        this._nextFreeSlot = 0;
        console.log("ProxyModelCoverageAngles constructor ended");
    },


    // If there is a free slot, we return that.
    // If not, the slot for the proxy model farthest away is returned.
    // It is assumed that the caller will make use of the slot, i.e., it will be flagged as in use after this call.
    bestSlot: function(model) {
        console.log("bestSlot: nextFreeSlot=" + this._nextFreeSlot + " ringSize=" + this._ringSize);
        if (this._nextFreeSlot<this._ringSize) {
            this._nextFreeSlot++;
            console.log("bestSlot: returning slot " + (this._nextFreeSlot-1));
            return this._nextFreeSlot-1;
        } else {
            var dir   = vec3.create( [-model.to_world[8], -model.to_world[9], -model.to_world[10]] );
            var bestAngle = vec3.dot( dir, this._direction[0] );
            console.log("bestSlot: first angle: " + bestAngle);
            var bestI = 0;
            for (i=1; i<this._ringSize; i++) {
                var cosAngle = vec3.dot( dir, this._direction[i] );
                console.log("bestSlot: angle for i=" + i + ": " + cosAngle);
                if (cosAngle<bestAngle) {
                    bestAngle = cosAngle;
                    bestI = i;
                }
            }
            return bestI;
        }
    },


    update: function(model, slot) {
        var d = vec3.create( [-model.to_world[8], -model.to_world[9], -model.to_world[10]] );
        console.log("update: Setting dir for slot i=" + slot + " to: " + d[0] + " " + d[1] + " " + d[2]);
        this._direction[slot] = vec3.create( [-model.to_world[8], -model.to_world[9], -model.to_world[10]] );
    },


});
