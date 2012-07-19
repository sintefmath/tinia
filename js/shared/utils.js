/* Copyright STIFTELSEN SINTEF 2012
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

tinia = {}

/** This is similar to dojo.hitch. It couples a function and an object, ensuring
  * that the this pointer in the function will be the object.
  * @param object the object to treat as the this pointer
  * @param fun the function to hitch
  */
tinia.hitch = function(object, fun) {
    if(fun.bind) {
        return fun.bind(object);
    } else {
        return function() {
            fun.apply(object, arguments);
        }
    }
}

/** Updates the objects width and heigth whenever the model element updates it.
  * @param key the key of the Viewer
  * @param exposedModel the associated ExposedModel
  * @param objectToStoreDimensions the object to set width and height on
  * @parma callbackFunction can be undefined. If valid, will be called whenever
  *                         width and height is updated.
 */
tinia.listenToDimensions = function(key, exposedModel, objectToStoreDimensions, callbackFunction) {
    exposedModel.addLocalListener(key, function(key, value) {
        if(objectToStoreDimensions.height != value.getElementValue("height") ||
                objectToStoreDimensions.width != value.getElementValue("width")) {
            objectToStoreDimensions.height = value.getElementValue("height");
            objectToStoreDimensions.width = value.getElementValue("width");
            if(callbackFunction) {
                callbackFunction();
            }
        }
    });
}
