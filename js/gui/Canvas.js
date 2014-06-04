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

dojo.declare("gui.Canvas", [dijit._Widget], {
    constructor: function (params) {

        this._useAutoProxy = true;

        if (!params.renderListURL) {
            params.renderListURL = "xml/getRenderList.xml";
        }
        if (!params.snapshotURL) {
            params.snapshotURL = "snapshot.txt";
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
        this._snapshotURL = params.snapshotURL;

        this._localMode = params.localMode;

        this._urlHandler.updateParams({
            "key": this._key
        });
        this._onLoadFunction = dojo.hitch(this, this._loadComplete);

        this._eventHandlers = Array();
        if (params.scripts) {
            for (var i = 0; i < params.scripts.length; i++) {
                var param = params.scripts[i].params();
                param.exposedModel = this._modelLib;
                this._eventHandlers[this._eventHandlers.length] = new (dojo.getObject(params.scripts[i].className()))(param);
            }
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
        dojo.connect(this._img, "onload", dojo.hitch(this, this._loadComplete()));

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
        this._shouldUpdate = true;

        this._urlHandler.setURL(this._snapshotURL);

        dojo.subscribe("/model/updateParsed", dojo.hitch(this, function (params) {

            if (!this._imageLoading) {
                console.log("Getting new image");
                dojo.xhrGet({
                    url: this._urlHandler.getURL(),
                    preventCache: true,
                    load: dojo.hitch(this, function (response, ioArgs) {
                        var response_obj = eval( '(' + response + ')' );
                        this._setImageFromText( response_obj.rgb, response_obj.depth, response_obj.view, response_obj.proj  );
                    })
                });
                // console.log("url: " + url);
            }

        }));

        dojo.subscribe("/model/updateSendStart", dojo.hitch(this, function (xml) {
            this._imageLoading = true;
            this._showCorrect();
        }));

        dojo.subscribe("/model/updateSendPartialComplete", dojo.hitch(this, function (params) {

            // Temporary sanity fix for firefox
            // this._setImageFromText(params.response); // @@@ Chrome gets here too. Should this be here? Would be nice to know why... Is this a bug workaround?

            if (params.response.match(/\"rgb\"\:/)) { // For the time being, we assume this to be an image.
                var response_obj = eval( '(' + params.response + ')' );
                this._setImageFromText( response_obj.rgb, response_obj.depth, response_obj.view, response_obj.proj );
            } else {
                console.log("This was not a snapshot. Why are we here at all?");
            }

        }));

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
        this._active = true;

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
        this._active = false;
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
        // What is this about?
        if (!response_rgb.substring(response_rgb.length - 1).match(/^[0-9a-zA-z\=\+\/]/)) {
            response_rgb = response_rgb.substring(0, response_rgb.length - 1);
        }
        this._img.src = "data:image/png;base64," + response_rgb;
        if (this._proxyRenderer) {
            this._proxyRenderer.setDepthData(response_rgb, response_depth, response_view, response_proj);
        }
        // This would show the image from the server, if it was the rgb-image.
        // When it is the depth buffer, it may look "funny".
        this._showCorrect();
        return response_rgb;
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
        this._active = true;

        var x = event.pageX - this._placementX();
        var y = event.pageY - this._placementY();
        event.relativeX = x;
        event.relativeY = y;
        for (var i = 0; i < this._eventHandlers.length; ++i) {
            this._eventHandlers[i].mousePressEvent(event);
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
        this._active = false;
        var x = event.pageX - this._placementX();
        var y = event.pageY - this._placementY();
        event.relativeX = x;
        event.relativeY = y;
        for (var i = 0; i < this._eventHandlers.length; ++i) {
            this._eventHandlers[i].mouseReleaseEvent(event);
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
            console.log("The browser should support WebGL.");
        } else {
            console.log("The browser doesn't seem to support WebGL.");
        }

        this._gl = WebGLUtils.setupWebGL(this._canvas);
        console.log("_startGL");
        if (this._gl) {

            if (!this._useAutoProxy) {
                // old proxy geometry
                this._render_list_store = new renderlist.RenderListStore(this._gl);
                this._render_list_parser = new renderlist.RenderListParser();
                this._render_list_renderer = new renderlist.RenderListRenderer(this._gl);
                this._getRenderList();
            } else {
                // new proxy geometry
                this._proxyRenderer = new gui.ProxyRenderer(this._gl, this._modelLib, this._key);
            }

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

        if (!this._useAutoProxy) {
            this._render_list_renderer.render(this._render_list_store, view_coord_sys);
        } else {
            this._proxyRenderer.render(view_coord_sys);
        }

    },


    _loadImage: function () {
        return;
        if (this._updatingBeforeLoad || this._localMode || this._active) {
            return;
        }

        console.log("trying to load image");
        this._loadImageAgain = 0;
        this._loadImageStage2();
    },

    _loadImageStage2: function () {
        this._DEBUG_imageLoadStart = (new Date()).getTime();
        this._imageLoading = true;
        this._showCorrect();
        //dojo.connect(this._img, "onload", this._onLoadFunction);
        // dojo.attr(this._img, "src", this._makeImgURL());
    },

    _loadComplete: function () {
        console.log("Time taken to load image: ", (new Date()).getTime() - this._DEBUG_imageLoadStart);
        this._imageLoading = false;

        if (this._loadImageAgain--) {
            this._loadImageStage2();
        } else {
        }

        this._showCorrect();
    },

    _makeImgURL: function () {
        return this._snapshotURL + "?width=" + this._width + "&height=" + this._height
        + "&timestamp=" + (new Date()).getTime() + "&key=" + this._key;
    },

    _update: function () {
        //if(this._localMode) return;

        this._showCorrect();
        this._loadImage();
    },

    _showCorrect: function () {

        if (this._localMode || (this._showRenderList && this._active) ||
            (this._imageLoading && this._mouseOver)) {
            dojo.style(this._img, "z-index", "0");
            this._img.style.zIndex = "0";
        }
//        else {
//            dojo.style(this._img, "z-index", "2");
//            this._img.style.zIndex = "2";
//        }

        if (this._loadingDiv) {
            if (this._imageLoading && !this._active) {
                this._loadingDiv.style.zIndex = "3";
                //  dojo.style(this._loadingDiv, "z-index", "3");
            }
            else {
                this._loadingDiv.style.zIndex = "0";
                //dojo.style(this._loadingDiv, "z-index", "0");
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
