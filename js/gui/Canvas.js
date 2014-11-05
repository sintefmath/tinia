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

dojo.provide("gui.Canvas");

dojo.require("3rdparty.webgl-utils");
dojo.require("renderlist.RenderList");
dojo.require("renderlist.RenderListParser" );
dojo.require("renderlist.RenderListRenderer" );
dojo.require("dijit._Widget");
dojo.require("gui.TrackBallViewer");
dojo.require("dojo.touch");
dojo.require("gui.ProxyRenderer");
dojo.require("gui.SnapshotTimings");



dojo.declare("gui.Canvas", [dijit._Widget], {


    constructor: function (params) {

        // Available snapshot signatures
        this._snapshotStrings = { png: "snapshot.txt", jpg: "jpg_snapshot.txt" ,ap: "snapshot_bundle.txt" };

        if (!params.renderListURL) {
            params.renderListURL = "xml/getRenderList.xml";
        }
        if (!params.snapshotBundleURL) {
            params.snapshotBundleURL = this._snapshotStrings.ap;
        }
        this._showRenderList = params.showRenderList;
        this._urlHandler = params.urlHandler;
        this._key = params.key;
        this._renderlistKey = params.renderlistKey;
        this._boundingboxKey = params.boundingboxKey;
        this._resetViewKey = params.resetViewKey;
        this._renderListURL = params.renderListURL;
        this._width = 1024;
        this._height = 1024;
        this._modelLib = params.modelLib;
        // The modification of fields of 'params' in this constructor is probably not necessary, because the call (there seems to be
        // only one) to the constructor uses a very short-lived automatic variable that is not used again before going out of scope.
        if (!params.snapshotURL) {
            if ( (this._modelLib.hasKey("ap_useJpgProxy")) && (this._modelLib.getElementValue("ap_useJpgProxy")) ) {
                if ( (this._modelLib.hasKey("ap_useAutoProxy")) && (this._modelLib.getElementValue("ap_useAutoProxy")) ) {
                    params.snapshotURL = this._snapshotStrings.ap;
                } else {
                    params.snapshotURL = this._snapshotStrings.jpg;
                }
            } else {
                params.snapshotURL = this._snapshotStrings.png;
            }
        }
        this._snapshotBundleURL = params.snapshotBundleURL;
        this._snapshotURL = params.snapshotURL;

        this._localMode = params.localMode;

        this._urlHandler.updateParams({
            "key": this._key
        });
        // It is possible that this new URL-parameter will (or should, maybe) make the old 'key' parameter obsolete?!
        this._urlHandler.addToParams({
            "viewer_key_list": this._key
        });
        if ( this._modelLib.hasKey("ap_jpgQuality") ) {
            this._urlHandler.updateParams({
                "jpeg_quality": this._modelLib.getElementValue("ap_jpgQuality")
            });
        } else {
            this._urlHandler.updateParams({
                "jpeg_quality": 50
            });
        }

        // This is not in use
        // this._onLoadFunction = dojo.hitch(this, this._loadComplete);

        this._eventHandlers = Array();
        if (params.scripts) {
            for (var i = 0; i < params.scripts.length; i++) {
                var param = params.scripts[i].params();
                param.exposedModel = this._modelLib;
                this._eventHandlers[this._eventHandlers.length] = new (dojo.getObject(params.scripts[i].className()))(param);
            }
        }

        // We must keep track of the timing details for the various snapshot methods
        this._snapshotTimings = new gui.SnapshotTimings();

        // Automatic adjustment of simulated latency every second
        setInterval(dojo.hitch(this, this._adjustSimulatedLatency), 1000);
    },


    _snapShotStringToType: function( url_type )
    {
        var snapType = "huh?";
        if ( url_type.substring(0, this._snapshotStrings.png.length) == this._snapshotStrings.png ) {
            snapType = "png";
        }
        else if ( url_type.substring(0, this._snapshotStrings.jpg.length) == this._snapshotStrings.jpg ) {
            snapType = "jpg";
        }
        else if ( url_type.substring(0, this._snapshotStrings.ap.length) == this._snapshotStrings.ap ) {
            snapType = "ap";
        }
        return snapType;
    },


    // When this is working, it should replace testing of both Exposed Model "magic" variables and current URLs (in this._urlHandler for instance.)
    // Modes: 0) png, 10-19) jpg with quality 0 to 90, 20) autoProxy ("ap")
    _getSnapshotMode: function() {
        if ( this._snapshotMode === undefined ) {
            // Has not yet been set, this means that we are entering autoSelect mode for the first time.
            // For this special case, we consult the ap_*-variables.
            if ( (this._modelLib.hasKey("ap_useAutoProxy")) && (this._modelLib.getElementValue("ap_useAutoProxy")) ) {
                // ap should override jpg
                this._snapshotMode = 20;
            }
            else if ( (this._modelLib.hasKey("ap_useJpgProxy")) && (this._modelLib.hasKey("ap_jpgQuality")) && (this._modelLib.getElementValue("ap_useJpgProxy")) ) {
                // "jpg" + parseInt(this._modelLib.getElementValue("ap_jpgQuality")/10) );
                this._snapshotMode = 10 + parseInt(this._modelLib.getElementValue("ap_jpgQuality")/10);
            }
            else {
                this._snapshotMode = 0; // png
            }
        }

        // We also produce a proper string and jpg-quality number for the current mode
        var currentSnapType = "huh?";
        var currentJpgQuality = -10;
        if ( this._snapshotMode == 0 ) {
            currentSnapType = "png";
        }
        else if ( (this._snapshotMode>=10) && (this._snapshotMode<20) ) {
            currentSnapType = "jpg";
            currentJpgQuality = (this._snapshotMode-10) * 10;
        }
        else if (this._snapshotMode==20) {
            currentSnapType = "ap";
        }
        else {
            throw "Should not happen. Unknown mode: " + this._snapshotMode;
        }

// @@@ Hvorfor funker ikke testen under når variabelen bare defineres i en av branchene over???!!! Får en merkelig oppførsel, ingen feilmelding.
//        if ( ! ( (currentJpgQuality==='undefined') || (currentJpgQuality===null) ) ) {
//            console.log(currentJpgQuality);
//        }

        return { mode: this._snapshotMode, snaptype: currentSnapType, jpgq: currentJpgQuality };
    },


    // @@@ Should be renamed to "setAutoSelectSnapshotMode", since this is to be used by that mode. It will for instance not be set to 'png' when a png is
    // requested at "mouse up".
    _setSnapshotMode: function(mode) {

        // temporary:
        // @@@ check now if it is safe to remove these, i.e., om removal breaks the autoselect mode

        if (mode==20) {
            // ap should override jpg
            if (this._modelLib.hasKey("ap_useAutoProxy")) {
                 this._modelLib.updateElement("ap_useAutoProxy", true);
            }
            if (this._modelLib.hasKey("ap_useJpgProxy")) {
                 this._modelLib.updateElement("ap_useJpgProxy", false);
            }
        }
        else if (mode==0) {
            // png
            if (this._modelLib.hasKey("ap_useAutoProxy")) {
                 this._modelLib.updateElement("ap_useAutoProxy", false);
            }
            if (this._modelLib.hasKey("ap_useJpgProxy")) {
                 this._modelLib.updateElement("ap_useJpgProxy", false);
            }
        }
        else {
            // jpg
            if (this._modelLib.hasKey("ap_useAutoProxy")) {
                 this._modelLib.updateElement("ap_useAutoProxy", false);
            }
            if (this._modelLib.hasKey("ap_useJpgProxy")) {
                 this._modelLib.updateElement("ap_useJpgProxy", true);
            }
            if (this._modelLib.hasKey("ap_jpgQuality")) {
                 this._modelLib.updateElement("ap_jpgQuality", (mode-10)*10);
            }
        }

        this._snapshotMode = mode;
    },


    _adjustSimulatedLatency: function() {
        if ( (this._modelLib.hasKey("simulatedAdditionalLatency")) && (this._modelLib.hasKey("simulatedAdditionalLatencyDecay")) ) {
            var ms = this._modelLib.getElementValue("simulatedAdditionalLatency");
            var delta = this._modelLib.getElementValue("simulatedAdditionalLatencyDecay");
            ms = ms + delta;
            if (ms<0) {
                ms = 0;
            }
            this._modelLib.updateElement("simulatedAdditionalLatency", ms);
        }
    },


    resize: function (w, h) {
        this._width = w;
        this._height = h;
        this._setWidthHeight(this.domNode);
        this._setAttrWidthHeight(this._img);
        this._setAttrWidthHeight(this._canvas);
        this._placeImage();
        this._urlHandler.updateParams({
            "width": w,
            "height": h
        });
        var viewer = this._modelLib.getValue(this._key);
        viewer.updateElement("width", w);
        viewer.updateElement("height", h);
        this._modelLib.updateElement(this._key, viewer);
    },


    // New attempt: We do not change type unless we have at least this._snapshotTimings._n timings of the current type.
    // Also: We only change type "one step in one direction" at a time, i.e., we go from jpg with quality q to q+delta or q-delta,
    // or we go from coarsest jpg to autoProxy, or from finest jpg to png. From png we go to finest jpg, and from autoProxy we go to
    // coarsest jpg.

    _autoSelectSnapshotType: function(timings) {
        if ( (this._modelLib.hasKey("ap_autoSelect")) && (this._modelLib.getElementValue("ap_autoSelect")) &&
                (this._modelLib.hasKey("ap_jpgQuality")) &&
                (this._modelLib.hasKey("ap_autoSelectTargetTime")) &&
                (this._modelLib.hasKey("ap_autoSelectTargetTimeSlack")) &&
                (this._mouseDownOrTouching) ) {

            // Setting up parameters for the current type of auto-selected snapshot
            var newJpgQuality = -10; // Just to catch bugs. Should never be allowed to be used
            var currentModeDetails = this._getSnapshotMode();
            var currentMode = currentModeDetails.mode;
            var currentSnapType = currentModeDetails.snaptype;
            var currentJpgQuality = currentModeDetails.jpgq;

            // Getting the various times to use
            var targetTime = this._modelLib.getElementValue("ap_autoSelectTargetTime");
            var targetTimeSlack = this._modelLib.getElementValue("ap_autoSelectTargetTimeSlack");
            var currentTime = 0;
            if ( currentSnapType == "jpg" ) {
                currentTime = this._snapshotTimings.getAvgTime( "jpg" + parseInt(currentJpgQuality/10) );
            } else {
                currentTime = this._snapshotTimings.getAvgTime( currentSnapType );
            }

            console.log("_autoSelectSnapshotType: currentSnapType = " + currentSnapType + " (jpg q = " + currentJpgQuality + "), currentTime = " + currentTime);
            if (currentTime==0) {
                console.log("  Returning early, we don't have timing information, yet...");
                return; // We don't want to change method until we have some more samples, so we just exit early.
            }

            var newSnapType = "huh?";

            // Testing if we should go faster, slower or stay
            // Note that we cannot use the simple time comparison to determine whether or not to go from "ap" to "jp0", since "ap" will
            // always be very slow in the timings, although snappy in feel.
            if ( currentSnapType == "ap" ) {
                // Special treatment.

                var apTimeAtTransitionFromJPG0 = timings.getRecordedAPtime();
                console.log("  We are in ap-mode, apTimeAtTransitionFromJPG0 (in reality, 5 frames after transition!) = " + apTimeAtTransitionFromJPG0 + ", and currentTime is still = " + currentTime);
                newSnapType = "ap";
                if ( apTimeAtTransitionFromJPG0 == 0 ) {
                    // We do not have at least timings._n ap-frames since the transition, so we do nothing
                }
                else if ( currentTime < apTimeAtTransitionFromJPG0 - 2*targetTimeSlack ) {
                    // console.log("We have enough frames, and time has been reduced, so we go up to jpg0");
                    // Ok, in this case, we go back to jpg0. The rationale behind this:
                    // We go down, from jpg0 to ap, when jpg0-time > targetTime + slack. After the first _n ap-frames,
                    // we get an ap-time, which we store and use for subsequent time comparisons, let this be named ap-time-0.
                    // We go up from ap to jpg0 again when ap-time < ap-time-0 - 2*slack, i.e., we have a time reduction
                    // approximately equal to 2*slack.
                    newSnapType = "jpg";
                    this._modelLib.updateElement("ap_jpgQuality", 0);
                    this._setSnapshotMode(10);
                } else {
                    // Do nothing. We stay with "ap", we don't have a faster alternative.
                    // console.log("Staying in ap");
                }
            } else {
                // We are in a position in which we can use the current timings, current snap type is either a "version" of jpg or png.

                if ( currentTime < targetTime-targetTimeSlack) {
                    // console.log("  We can afford to go for higher quality, i.e., slower");
                    if ( currentSnapType == "ap" ) {
                        alert("should not happen");
                    }
                    else if ( currentSnapType == "jpg" ) {
                        if ( currentJpgQuality < 90 ) {
                            newSnapType = "jpg";
                            newJpgQuality = currentJpgQuality + 10;
                            this._modelLib.updateElement("ap_jpgQuality", newJpgQuality);
                            this._setSnapshotMode(10 + parseInt(newJpgQuality/10));
                        } else {
                            newSnapType = "png";
                            this._setSnapshotMode(0);
                        }
                    }
                    else {
                        newSnapType = "png"; // best we have... no change
                        this._setSnapshotMode(0);
                    }
                }
                else if ( currentTime > targetTime+targetTimeSlack ) {
                    // console.log("  We should go for faster method");
                    if ( currentSnapType == "png" ) {
                        newSnapType = "jpg";
                        newJpgQuality = 90;
                        this._modelLib.updateElement("ap_jpgQuality", newJpgQuality);
                        this._setSnapshotMode(19);
                    }
                    else if ( currentSnapType == "jpg" ) {
                        if ( currentJpgQuality >= 10 ) {
                            newSnapType = "jpg";
                            newJpgQuality = currentJpgQuality - 10;
                            this._modelLib.updateElement("ap_jpgQuality", newJpgQuality);
                            this._setSnapshotMode(10 + parseInt(newJpgQuality/10));
                        } else {
                            newSnapType = "ap";
                            // Switching from jpg0 to ap, must initiate the ap-time-recording. We need this to determine when to leave ap again.
                            timings.initiateAPrecording();
                        }
                    }
                    else {
                        alert("should not happen 2");
                    }
                }
                else {
                    console.log("  time within tolerances, not changing snaptype");
                    newSnapType = currentSnapType;
                    newJpgQuality = currentJpgQuality;
                }
            }


            if (this._modelLib.hasKey("ap_autoSelectIndicator")) {
                if ( newSnapType == "jpg" ) {
                    this._modelLib.updateElement( "ap_autoSelectIndicator", newSnapType + parseInt(newJpgQuality/10) );
                } else {
                    this._modelLib.updateElement( "ap_autoSelectIndicator", newSnapType );
                }
            }



            // console.log("_autoSelectSnapshotType done. New snapType = " + newSnapType);

            if ( newSnapType != "huh?" ) {
                this._snapshotURL = this._snapshotStrings[newSnapType];
                this._urlHandler.setURL(this._snapshotURL);
                this._urlHandler.updateParams( { snaptype: newSnapType } );
                // console.log("--------------- updating url params with new snaptype " + newSnapType + " ---------------------");
                if ( newSnapType == "ap" ) {
                    this._modelLib.updateElement( "ap_useAutoProxy", true );
                    // Need this to make sure proxy is rendered during mouse-down. Can this have other, unwanted effects? If so,
                    // this must be changed. (I.e., autoProxy-rendering must not test on this variable (only.)
                }
            }
        }
    },


    _requestImageIfNotBusy: function() {
        if (!this._imageLoading) {
            // console.log("_requestImageIfNotBusy (/model/updateParsed or mouseUp): Getting new image, url=" + this._urlHandler.getURL());
            var startTime = Date.now();

            // Should this be done? It is done in ExposedModelSender._makeURL(), which is used when posting with dojo.rawXhrPost()!
            this._urlHandler.updateParams( { "revision" : this._modelLib.getRevision(), "timestamp" : (new Date()).getTime() } );
            var url_used = this._urlHandler.getURL();


            dojo.xhrGet({ // Here we explicitly ask for a new image in a new HTTP connection.
                            url: this._urlHandler.getURL(),
                            preventCache: true,
                            load: dojo.hitch(this, function (response, ioArgs) {
                                var t0 = Date.now();
                                // console.log("_requestImageIfNotBusy: response = " + response);
                                var response_obj = eval( '(' + response + ')' );
                                // console.log("/model/updateParsed: response[" + this._key + "].view = " + response_obj[this._key].view);
                                // console.log("/model/updateParsed: response[" + this._key + "].proj = " + response_obj[this._key].proj);
                                this._setImageFromText( response_obj[this._key].rgb, response_obj[this._key].depth, response_obj[this._key].view, response_obj[this._key].proj );
                                var snaptype = response_obj[this._key].snaptype;
                                if (response_obj[this._key].snaptype == "jpg") {
                                    snaptype = snaptype + parseInt(this._modelLib.getElementValue("ap_jpgQuality")/10);
                                    // @@@ ARGH! This is not correct, we should also record the quality in the url and response!!!
                                }
                                console.log("_requestImageIfNotBusy: receieved snaptype = " + snaptype);
                                this._snapshotTimings.update( snaptype, (t0 - response_obj[this._key].timestamp) );
                                this._snapshotTimings.print();
                                this._autoSelectSnapshotType(this._snapshotTimings);
                            })
                        });
        }
    },


    buildRendering: function () {
        this.domNode = dojo.create("div");
        dojo.style(this.domNode, "background", "black");
        dojo.style(this.domNode, "margin", "10px");
        dojo.style(this.domNode, "padding", "0px");

        dojo.addClass(this.domNode, "unselectable");

        this._setWidthHeight(this.domNode);
        this._canvas = dojo.create("canvas");
        dojo.style(this._canvas, "position", "relative");
        dojo.style(this._canvas, "float", "none");
        dojo.style(this._canvas, "padding", 0);
        dojo.style(this._canvas, "margin", 0);
        dojo.style(this._canvas, "display", "block");
        this._canvas.style.zIndex = "1";
        dojo.style(this._canvas, "z-index", "1");

        this._setAttrWidthHeight(this._canvas);

        if (window.WebGLRenderingContext) {
            this.domNode.appendChild(this._canvas);
        }

        this._img = dojo.create("img");
        dojo.attr(this._img, "src", "image/canvas_backdrop.jpg");
        dojo.attr(this._img, "alt", "Image from server");
        dojo.style(this._img, "position", "relative");
        dojo.style(this._img, "float", "none");
        dojo.style(this._img, "z-index", "2");

        dojo.style(this._img, "padding", 0);
        dojo.style(this._img, "margin", 0);

        // 141024: This syntax is buggy, the function is not registered, but no error is induced either! Commenting this out.
        // dojo.connect(this._img, "onload", dojo.hitch(this, this._loadComplete()));

        this._setAttrWidthHeight(this._img);

        this.domNode.appendChild(this._img);

        // Create loading-div
        this._loadingDiv = dojo.create("div");
        dojo.style(this._loadingDiv, "position", "relative");
        dojo.style(this._loadingDiv, "float", "none");
        dojo.style(this._loadingDiv, "z-index", "3");


        dojo.style(this._loadingDiv, "padding", 0);
        dojo.style(this._loadingDiv, "margin", 0);

        dojo.addClass(this._loadingDiv, "unselectable");

        this._loadingText = dojo.create("span");
        this._loadingText.innerHTML = "Loading new image...";
        dojo.style(this._loadingText, "margin-left", "10px");
        dojo.style(this._loadingText, "margin-top", "10px");

        dojo.style(this._loadingText, "font-size", "2em");
        dojo.style(this._loadingText, "color", "#FF8B8B");

        dojo.style(this._loadingText, "font-weight", "bold");
        this._loadingDiv.appendChild(this._loadingText);

        this._setWidthHeight(this._loadingDiv);
        this.domNode.appendChild(this._loadingDiv);

        this._placeImage();
        this._update();

        this._modelLib.addLocalListener(this._renderlistKey, function (key, value) {
            this._getRenderList();
        }, this);

        // We always want to update after the first parsing
        // This variable is never referenced by our code. Is it something forgotten and obsolete?
        this._shouldUpdate = true;

        if ( (this._modelLib.hasKey("ap_useAutoProxy")) && (this._modelLib.getElementValue("ap_useAutoProxy")) ) {
            this._urlHandler.setURL(this._snapshotBundleURL);
        } else {
            this._urlHandler.setURL(this._snapshotURL);
        }
        this._urlHandler.updateParams({snaptype: this._snapShotStringToType( this._urlHandler.getURL() )});
        this._modelLib.addLocalListener( "ap_useAutoProxy", dojo.hitch(this, function(event) {
            if ( this._modelLib.getElementValue("ap_useAutoProxy") ) {
                this._urlHandler.setURL(this._snapshotBundleURL);
            } else {
                this._urlHandler.setURL(this._snapshotURL);
            }
            this._urlHandler.updateParams({snaptype: this._snapShotStringToType( this._urlHandler.getURL() )});
        }) );
        this._modelLib.addLocalListener( "ap_useJpgProxy", dojo.hitch(this, function(event) {
            if ( ! ( (this._modelLib.hasKey("ap_useAutoProxy")) && (this._modelLib.getElementValue("ap_useAutoProxy")) ) ) {
                // We have decided to let ap_useAutoProxy override ap_useJpgProxy
                if ( this._modelLib.getElementValue("ap_useJpgProxy") ) {
                    this._snapshotURL = this._snapshotStrings.jpg;
                } else {
                    this._snapshotURL = this._snapshotStrings.png;
                }
            } else {
                // If we get here, jpg mode has just been toggled, while ap mode has been on, i.e., this._snapshotURL should
                // already have been set to "snapshot_bundle.txt".
                // Hmm. This seems not to be true. How can it be set to "snapshot.txt", when ap mode should be taking precedence??!!
                // Aha. Probably because the "mouse-up" routine does this.
                // No. Shouldn't be that.
                // Seems that when we are in JPG-mode, and switch on AP mode, the mouse-down-up cycle happens (makes sense) and on the mouse-up-exit
                // the url is set to be "snapshot.txt", since we are detected to not be in AP mode (this probably happens before the exposed model
                // round trip with the element for this) and also releasing the mouse. Thus, we end up in AP mode with url=snapshot.txt.
                // Should probably be ok, but we then need to set it here, after all...
                // console.log("old url was: " + this._snapshotURL);
                this._snapshotURL = this._snapshotStrings.ap;
            }
            this._urlHandler.setURL(this._snapshotURL);
            this._urlHandler.updateParams({snaptype: this._snapShotStringToType( this._urlHandler.getURL() )});
        }) );
        this._modelLib.addLocalListener( "ap_jpgQuality", dojo.hitch(this, function(event) {
            this._urlHandler.updateParams( { "jpeg_quality": this._modelLib.getElementValue("ap_jpgQuality") } );
        }) );

        this._modelLib.addLocalListener( "ap_autoSelectSampleAll", dojo.hitch(this, function(event) {
            if ( this._modelLib.getElementValue("ap_autoSelectSampleAll") ) {
                console.log("Should now sample all snapshot types to obtain timing results");

                for (var k=0; k<this._snapshotTimings._n; k++) { // To fill the ring buffers used for averaging
                    for (var i=0; i<10; i++) {
                        this._snapshotTimings.update( "jpg" + i, 50 + 5 * i );
                    }
                    this._snapshotTimings.update( "png", 100 );
                    this._snapshotTimings.update( "ap", 200 );
                }
                this._snapshotTimings.print();

            }
        }) );



        // Some timing issues:
        //
        // Note that "updateSendStart" is generated when the client sends an update to the server. This is not the same as the client asking for a new image,
        // which is actually what happens when the *server* sends an update to the *client*. When this happens, "updateParsed" is generated.
        // (Why do we observe "updateParsed" so often then, for a dumb server not initiating much at all, just rendering a static scene?)
        //
        // If we update the server (will happen during mouse activity, the "mouse-move events" are sent as such updates?!) an "updateSendStart" is
        // produced. This happens for instance when the mouse button is pressed. If the mouse is moved at once, and continuously, we get a stream of
        // updates from the client to the server, accompanied by a stream of "updateSendPartialComplete". When things settle down (mouse kept still or
        // button released) we get an "updateSendComplete".
        //
        // In order to time the production and sending of an image from the server, it seems that the safest way is to measure the time from "updateSendStart"
        // to "updateSendPartialComplete" and/or "updateSendComplete" for the first image, and from "updateSendPartialComplete" to "updateSendPartialComplete"
        // and/or "updateSendComplete" for subsequent images. (It seems we get an "updateSendComplete" immediately after an "updateSendPartialComplete", so that
        // the time between the last "updateSendPartialComplete" and the final "updateSendComplete" is not so interesting...)
        //
        // Note that we cannot rely on timing the xhrGet() call from issuing it to its load-method completion, since so many received images are not the
        // result of a server-initiated image production.
        //
        // Note also that one advantage of using "updateSendPartialComplete" to mark the end of a timed interval is that this ensures that we are in a
        // continuous stream of snapshots, meaning that we have less probability of a "singular" snapshot taking additional time for instance in connection
        // with long-polling or something, or the opposite, like in the case of the "updateSendComplete".

        // There *may* be a problem with how URLs are handled, specifically in how the URLs are set in the URLHandler object.
        // The problem is that this has gotten a bit convoluted, and that the URL and the other fields (like revision and timestamp for instance)
        // are not set at the same time. This makes it very bug-prone to examine the URLHandler state to deduce anything wrt. the received responses!
        // A cleanup would be nice, but the problem could also be alleviated by relying on comparing fields in URL requested and snapshot received.
        // (Will do this (first) for timing purposes.)






        // This gets called when the SERVER has initiated a change in the model.
        // This could for instance be a simulation and the server updates the current
        // simulation time.
        // We need to fetch a new image.
        dojo.subscribe("/model/updateParsed", dojo.hitch(this, function (params) {
            this._requestImageIfNotBusy();
        }));

        // We are initiating a new update TO the server.
        dojo.subscribe("/model/updateSendStart", dojo.hitch(this, function (xml) {
            this._imageLoading = true;
            // console.log("/model/updateSendStart: ******************* url = " + this._urlHandler.getURL());
            // Make sure we are showing the proxy geometry (if the mouse is over the canvas):
            this._showCorrect(); // Shows the correct image: either proxy or server image.
        }));

        // We have sent a new update to the server.
        // The "partial" bit says that we _may_ have another update to send after this as well.
        // Either way, we should show the image we just got from the server (it's newer than the one we have!).
        dojo.subscribe("/model/updateSendPartialComplete", dojo.hitch(this, function (params) {
            if (params.response.match(/\"rgb\"\:/)) { // For the time being, we assume this to be an image.
                // console.log("/model/updateSendPartialComplete: response = " + params.response);
                var response_obj = eval( '(' + params.response + ')' );
//                console.log("/model/updateSendPartialComplete: response[" + this._key + "].view = " + response_obj[this._key].view);
//                console.log("/model/updateSendPartialComplete: response[" + this._key + "].proj = " + response_obj[this._key].proj);
                if (response_obj) { // 140616: Suddenly, params.response seems to be an empty string, from time to time, requiring this
                    this._setImageFromText( response_obj[this._key].rgb, response_obj[this._key].depth, response_obj[this._key].view, response_obj[this._key].proj );
                    var tmp = Date.now();
                    var snaptype = response_obj[this._key].snaptype;
                    if (response_obj[this._key].snaptype == "jpg") {
                        snaptype = snaptype + parseInt(this._modelLib.getElementValue("ap_jpgQuality")/10);
                    }
                    console.log("/model/updateSendPartialComplete: received snaptype = " + snaptype);
                    this._snapshotTimings.update( snaptype, (tmp - response_obj[this._key].timestamp) );
                    this._snapshotTimings.print();
                    this._autoSelectSnapshotType(this._snapshotTimings);
                }
            } else {
                console.log("This was not a snapshot. Why are we here at all?");
            }
        }));

        // Here we know that we have no further updates to send at the moment. Therefore the
        // image we received is a perfect match for our current exposedmodel.
        dojo.subscribe("/model/updateSendComplete", dojo.hitch(this, function (params) {
            this._imageLoading = false;
            this._showCorrect();
        }));

        this.resize(this._width, this._height);
        this._updateMatrices();
    },


    startup: function () {
        this._startGL();
        this.on("mousedown", dojo.hitch(this, this._mousedown));
        //	this.domNode.addEventListener("touchstart",  dojo.hitch(this, this._mousedown));
        //	this._img.addEventListener("touchstart", dojo.hitch(this, this._mousedown));
        dojo.connect(document, "onmouseup", dojo.hitch(this, this._mouseup));
        //	this.on("touchend", dojo.hitch(this, this._mouseup));
        this.on("mousemove", dojo.hitch(this, this._mousemove));
        this.domNode.addEventListener("touchstart", dojo.hitch(this, this._touchstart));
        this.domNode.addEventListener("touchend", dojo.hitch(this, this._touchend));
        this.domNode.addEventListener("touchmove", dojo.hitch(this, this._touchmove));
        this.on("mouseover", dojo.hitch(this, function () {
            this._mouseOver = true;
            this._showCorrect();
        }));
        this.on("mouseout", dojo.hitch(this, function () {
            this._mouseOver = false;
            this._showCorrect();
        }));

        //        document.addEventListener("gesturestart", dojo.hitch(this, this._touchGestureBegin));
        //        document.addEventListener("gesturechange", dojo.hitch(this, this._touchGesture));

        // Use these to prevent rightclicking on the image (we need this to enable extra mouse events)
        dojo.connect(this._img, "oncontextmenu", function (e) {
            e.preventDefault();
        });
        this.on("contextmenu", function (event) {
            event.preventDefault();
            return false;
        });

        // Resizing.
        dojo.connect(document, "onmousedown", dojo.hitch(this, this._mouseDownResize));
        dojo.connect(document, "onmouseup", dojo.hitch(this, this._mouseUpResize));
        dojo.connect(document, "onmousemove", dojo.hitch(this, this._mouseMoveResize));

        // Keyboard press
        this.domNode.setAttribute("tabindex", 0);
        this.on("mousedown", dojo.hitch(this, function (event) {
            this.domNode.focus();
        }));
        this.on("keydown", dojo.hitch(this, this._keyPress));

    },


    _touchstart: function (event) {
        this._mouseDownOrTouching = true;
        this._touch_prev = event;
        // We need to add the relative placement informatoin to all touch events
        for(var i = 0; i < event.touches.length; ++i) {
            var x = event.touches[i].pageX - this._placementX();
            var y = event.touches[i].pageY - this._placementY();
            event.touches[i].relativeX = x;
            event.touches[i].relativeY = y;
        }
        event.pageX = event.touches[0].pageX;
        event.pageY = event.touches[0].pageY;
        event.relativeX = event.touches[0].relativeX;
        event.relativeY = event.touches[0].relativeY;
        for (var i = 0; i < this._eventHandlers.length; ++i) {
            if (this._eventHandlers[i].touchStartEvent) {
                this._eventHandlers[i].touchStartEvent(event);
            }
        }
        this._showCorrect();
        event.preventDefault();
    },


    _touchend: function (event) {
        this._mouseDownOrTouching = false;
        if( event.touches.length == 0 ) {
            // Use last active position
            event = this._touch_prev;
        }
        for(var i = 0; i < event.touches.length; ++i) {
            var x = event.touches[i].pageX - this._placementX();
            var y = event.touches[i].pageY - this._placementY();
            event.touches[i].relativeX = x;
            event.touches[i].relativeY = y;
        }
        // We need to add the relative placement informatoin to all touch events
        event.pageX = event.touches[0].pageX;
        event.pageY = event.touches[0].pageY;
        event.relativeX = event.touches[0].relativeX;
        event.relativeY = event.touches[0].relativeY;
         for (var i = 0; i < this._eventHandlers.length; ++i) {
            if (this._eventHandlers[i].touchEndEvent) {
                this._eventHandlers[i].touchEndEvent(event);
            }
        }
        this._showCorrect();
        event.preventDefault();
    },


    _touchmove: function (event) {
        this._touch_prev = event;
        for(var i = 0; i < event.touches.length; ++i) {
            var x = event.touches[i].pageX - this._placementX();
            var y = event.touches[i].pageY - this._placementY();
            event.touches[i].relativeX = x;
            event.touches[i].relativeY = y;
        }
        event.pageX = event.touches[0].pageX;
        event.pageY = event.touches[0].pageY;
        event.relativeX = event.touches[0].relativeX;
        event.relativeY = event.touches[0].relativeY;
        for (var i = 0; i < this._eventHandlers.length; ++i) {
            if (this._eventHandlers[i].touchMoveEvent) {
                this._eventHandlers[i].touchMoveEvent(event);
            }
        }
        event.preventDefault();
    },


    _keyPress: function (event) {
        if (event.key === undefined) {
            event.key = event.keyCode ? event.keyCode : event.charCode;
        }
        for (var i = 0; i < this._eventHandlers.length; ++i) {
            if (this._eventHandlers[i].keyPressEvent) {
                this._eventHandlers[i].keyPressEvent(event);
            }
        }
    },


    _placeImage: function () {
        this._img.style.top = (-this._height) + "px";
        this._loadingDiv.style.top = (-2 * this._height) + "px";
    },


    _setImageFromText: function (response_rgb, response_depth, response_view, response_proj) {
        if (!response_rgb.substring(response_rgb.length - 1).match(/^[0-9a-zA-z\=\+\/]/)) { // @@@ What is this about?
            response_rgb = response_rgb.substring(0, response_rgb.length - 1);
        }
        this._img.src = "data:image/png;base64," + response_rgb;
        if ( (this._modelLib.hasKey("ap_useAutoProxy")) && (this._modelLib.getElementValue("ap_useAutoProxy")) ) {
            if (this._proxyRenderer) {
                this._proxyRenderer.setDepthData(response_rgb, response_depth, response_view, response_proj);
            }
        } else {
            if ( (this._modelLib.hasKey("ap_useJpgProxy")) && (this._modelLib.getElementValue("ap_useJpgProxy")) ) {
                this._img.src = "data:image/jpg;base64," + response_rgb;
            }
        }
        this._showCorrect();
    },


    _touchGestureBegin: function (event) {
        if (event && event.scale) {
            this._trackball.zoomFactorBegin(event.scale);
        }
        event.preventDefault();
    },


    _touchGesture: function (event) {
        if (event && event.scale && event.scale != 1) {
            this._trackball.zoomFactor(event.scale);
            this._updateMatrices();
            this._showCorrect();
        }
        event.preventDefault();
    },


    _mousedown: function (event) {
        this._mouseDownOrTouching = true;
        var x = event.pageX - this._placementX();
        var y = event.pageY - this._placementY();
        event.relativeX = x;
        event.relativeY = y;
        for (var i = 0; i < this._eventHandlers.length; ++i) {
            this._eventHandlers[i].mousePressEvent(event);
        }
        if ( ! ( (this._modelLib.hasKey("ap_useAutoProxy")) && (this._modelLib.getElementValue("ap_useAutoProxy")) ) ) {
            if ( (this._modelLib.hasKey("ap_useJpgProxy")) && (this._modelLib.getElementValue("ap_useJpgProxy")) ) {
                this._snapshotURL = this._snapshotStrings.jpg;
                this._urlHandler.setURL(this._snapshotURL);
                this._urlHandler.updateParams({snaptype: this._snapShotStringToType( this._urlHandler.getURL() )});
                // console.log("Mouse down: Setting JPG mode, new URL=" + this._urlHandler.getURL());
            }
        }
        this._showCorrect();
        event.preventDefault();
    },


    _placementX: function () {
        return dojo.position(this._canvas).x;
    },


    _placementY: function () {
        return dojo.position(this._canvas).y;
    },


    _mouseup: function (event) {
        console.log("================================================ mouse up ========================================================");
        this._mouseDownOrTouching = false;
        var x = event.pageX - this._placementX();
        var y = event.pageY - this._placementY();
        event.relativeX = x;
        event.relativeY = y;
        for (var i = 0; i < this._eventHandlers.length; ++i) {
            this._eventHandlers[i].mouseReleaseEvent(event);
        }
        if ( (this._modelLib.hasKey("ap_autoSelect")) && (this._modelLib.getElementValue("ap_autoSelect")) ) {
            // When the autoSelect mode is enabled, we want an additional, and final, snapshot from the server, which should be of the png kind.
            this._snapshotURL = this._snapshotStrings.png;
            this._urlHandler.setURL(this._snapshotURL);
            this._urlHandler.updateParams({snaptype: this._snapShotStringToType( this._urlHandler.getURL() )});
            console.log("--------------- updating url with snaptype " + this._snapShotStringToType( this._urlHandler.getURL() ) + " ---------------------");
            this._requestImageIfNotBusy();
            // @@@ It happens from time to time that we do not get a final png. To trigger, move mouse while releasing. Not consistent, but happens more often then.
            // @@@ Could the reason be that this issued request is canceled due to "being busy"...?! So that the "jpg snapshot in flight" takes precedence?
        }

        this._showCorrect();
        event.preventDefault();
    },


    _mousemove: function (event) {
        var x = event.pageX - this._placementX();
        var y = event.pageY - this._placementY();
        event.relativeX = x;
        event.relativeY = y;
        for (var i = 0; i < this._eventHandlers.length; ++i) {
            this._eventHandlers[i].mouseMoveEvent(event);
        }
        event.preventDefault();
    },


    _updateMatrices: function () {
        return;
        var viewer = this._modelLib.getValue(this._key);
        this._imageLoading = true;
        this._modelLib.updateElement(this._key, viewer);
    },


    _setAttrWidthHeight: function (node) {
        dojo.attr(node, "width", this._width + "px");
        dojo.attr(node, "height", this._height + "px");
    },


    _setWidthHeight: function (node) {
        dojo.style(node, "width", this._width + "px");
        dojo.style(node, "height", this._height + "px");
    },


    _startGL: function () {
        if (window.WebGLRenderingContext) {
            // browser supports WebGL
            // console.log("The browser should support WebGL.");
        } else {
            console.log("The browser doesn't seem to support WebGL. Have you turned on necessary flags? Recent enough version? Chrome and Firefox should work.");
        }
        this._gl = WebGLUtils.setupWebGL(this._canvas);
        if (this._gl) {
            this._proxyRenderer = new gui.ProxyRenderer(this._gl, this._modelLib, this._key);
            this._render_list_store = new renderlist.RenderListStore(this._gl);
            this._render_list_parser = new renderlist.RenderListParser();
            this._render_list_renderer = new renderlist.RenderListRenderer(this._gl);
            this._getRenderList();
            this._render();
        } else {
            console.log("The browser does support WebGL, but we were unable to get a GL context. It may help to completely quit the browser, and restart it.");
        }
    },


    _getRenderList: function () {
        if (this._render_list_store === undefined) {
            return;
        }
        dojo.xhrGet(
        {
            url: this._renderListURL + "?key=" + this._key + "&timestamp=" + this._render_list_store.revision(),
            handleAs: "xml",
            timeout: 10000,
            load: dojo.hitch(this, function (data, ioargs) {
                this._render_list_parser.parse(this._render_list_store, data);
                this._updateMatrices();
                this._render();
                return data;
            })
        }
        );
    },


    _render: function () {
        window.requestAnimFrame(dojo.hitch(this, function () {
            this._render();
        }));
        var viewer = this._modelLib.getElementValue(this._key);
        var view_coord_sys = {
            m_projection: viewer.getElementValue("projection"),
            m_projection_inverse: mat4.inverse(mat4.create(viewer.getElementValue("projection"))),
            m_to_world: mat4.inverse(mat4.create(viewer.getElementValue("modelview"))),
            m_from_world: viewer.getElementValue("modelview")
        }
        if ( (this._modelLib.hasKey("ap_useAutoProxy")) && (this._modelLib.getElementValue("ap_useAutoProxy")) ) {
            this._proxyRenderer.render(view_coord_sys);
        } else {
            if ( (this._modelLib.hasKey("ap_useJpgProxy")) && (this._modelLib.getElementValue("ap_useJpgProxy")) ) {
                // console.log("Jpg-proxy! Should not do anything at all...")
            } else {
                // Neither autoProxy nor jpgProxy, fall back to render list
                this._render_list_renderer.render(this._render_list_store, view_coord_sys);
            }
        }
    },


    _update: function () {
        //if(this._localMode) return;
        this._showCorrect();
    },


    _showCorrect: function () {
        if ( (this._localMode) || (this._showRenderList && this._mouseDownOrTouching) || (this._imageLoading ) ) {
            dojo.style(this._img, "z-index", "0");
            this._img.style.zIndex = "0";
            // We will always get here, while holding a mouse button down inside the canvas.
            // Also when the mouse is inside the canvas and a button is pushed.
        } else {
            dojo.style(this._img, "z-index", "2");
            this._img.style.zIndex = "2";
            // We get here when the mouse is crossing the border to the canvas while no button is pressed.
            // Also when the mouse is inside and a button is released.
        }
        if (this._loadingDiv) {
            if (this._imageLoading && !this._mouseDownOrTouching) {
                this._loadingDiv.style.zIndex = "3";
                //  dojo.style(this._loadingDiv, "z-index", "3");
                // Have only seen this occasionally when double-clicking inside the canvas. Not consistently.
            } else {
                this._loadingDiv.style.zIndex = "0";
                //dojo.style(this._loadingDiv, "z-index", "0");
                // We get here while button is pressed and mouse is moving inside the canvas.
                // Also when crossing the canvas border, irrespective of button state.
            }
        }
    },


    _mouseMoveResize: function (event) {
        var x = event.pageX - this._placementX();
        var y = event.pageY - this._placementY();
        if (this._isResizing) {
            this.resize(x, y);
        }
    },


    _mouseDownResize: function (event) {
        if (event.button != 0) return;
        var x = event.pageX - this._placementX();
        var y = event.pageY - this._placementY();
        if (Math.max(Math.abs(this._width - x), Math.abs(this._height - y)) < 20) {
            this._isResizing = true;
        }
    },


    _mouseUpResize: function (event) {
        this._isResizing = false;
    }


})
