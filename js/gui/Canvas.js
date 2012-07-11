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

dojo.declare("gui.Canvas", [dijit._Widget], {
    constructor: function(params) {

        if(!params.renderListURL) {
            params.renderListURL = "xml/getRenderList.xml";
        }
        if(!params.snapshotURL) {
            params.snapshotURL = "snapshot.txt";
        }
	this._showRenderList = params.showRenderList;
        this._urlHandler = params.urlHandler;
        this._key = params.key;
        this._renderlistKey = params.renderlistKey;
        this._boundingboxKey = params.boundingboxKey;
        this._resetViewKey = params.resetViewKey;
        this._renderListURL = params.renderListURL;
        this._width = 500;
        this._height = 500;
        this._modelLib = params.modelLib;
        this._snapshotURL = params.snapshotURL;

        this._localMode = params.localMode;

        this._urlHandler.updateParams({"key": this._key});
        this._onLoadFunction = dojo.hitch(this, this._loadComplete);
        
        this._eventHandlers = Array();
        if(params.scripts) {
            for(var i = 0; i < params.scripts.length; i++) {
                var param = params.scripts[i].params();
                param.exposedModel = this._modelLib; 
                this._eventHandlers[this._eventHandlers.length] = new (dojo.getObject(params.scripts[i].className()))(param);
           }
        }
    },

    resize: function(w, h) {
        this._width = w;
        this._height = h;
        this._setWidthHeight(this.domNode);
        this._setAttrWidthHeight(this._img);
        this._setAttrWidthHeight(this._canvas);
        this._placeImage();

        this._urlHandler.updateParams({
            "width" : w,
            "height" : h
        });



        var viewer = this._modelLib.getValue(this._key);
        viewer.updateElement("width", w);
        viewer.updateElement("height", h);
        this._modelLib.updateElement(this._key, viewer);
    },

    buildRendering: function() {
        this.domNode = dojo.create("div");
        dojo.style(this.domNode, "background", "black");
        dojo.style(this.domNode, "margin", "10px");
        dojo.style(this.domNode, "padding", "0px");


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
        
        if( window.WebGLRenderingContext ) {
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
        this._placeImage();
        dojo.connect(this._img, "onload", dojo.hitch(this, this._loadComplete()));

        this._setAttrWidthHeight(this._img);

       this.domNode.appendChild(this._img);

        this._update();


        this._modelLib.addLocalListener(this._renderlistKey, function(key, value) {
            this._getRenderList();
        }, this);


        // We always want to update after the first parsing
        this._shouldUpdate = true;
        
        this._urlHandler.setURL(this._snapshotURL);
        
        dojo.subscribe("/model/updateParsed", dojo.hitch(this, function(params) {
            if(this._shouldUpdate) {
                this._updateMatrices();
            }
            this._shouldUpdate = false;
        }));

        dojo.subscribe("/model/updateSendPartialComplete", dojo.hitch(this, function(params) {
            // Temporary sanity fix for firefox
            if( ! params.response.substring(params.response.length-1).match(/^[0-9a-zA-z\=\+\/]/)) {
                params.response = params.response.substring(0, params.response.length-1);
            }
            this._img.src="data:image/png;base64,"+params.response;
            this._imageLoading = false;
            this._showCorrect();
        }));

        this.resize(this._width, this._height);
        this._updateMatrices();
    },



    startup: function() {
        this._startGL();
        this.on("mousedown", dojo.hitch(this, this._mousedown));
//	this.domNode.addEventListener("touchstart",  dojo.hitch(this, this._mousedown));
//	this._img.addEventListener("touchstart", dojo.hitch(this, this._mousedown));
        dojo.connect(document,"onmouseup", dojo.hitch(this, this._mouseup));
//	this.on("touchend", dojo.hitch(this, this._mouseup));
        this.on("mousemove", dojo.hitch(this, this._mousemove));
//	document.addEventListener("touchmove", dojo.hitch(this, this._mousemove));
        this.on("mouseover", dojo.hitch(this, function() {
            this._mouseOver = true;
        }));
        this.on("mouseout", dojo.hitch(this, function() {
            this._mouseOver = false;
        }));
  
//        document.addEventListener("gesturestart", dojo.hitch(this, this._touchGestureBegin));
//        document.addEventListener("gesturechange", dojo.hitch(this, this._touchGesture));

        dojo.connect(document,"onmousedown", dojo.hitch(this, this._mouseDownResize));
        dojo.connect(document,"onmouseup", dojo.hitch(this, this._mouseUpResize));
        dojo.connect(document, "onmousemove", dojo.hitch(this, this._mouseMoveResize));
        

    },



    _placeImage : function() {
        this._img.style.top = (-this._height)+"px";
    },
    
    
    _touchGestureBegin : function(event) {

        if(event && event.scale) {

            this._trackball.zoomFactorBegin(event.scale);

        }
        event.preventDefault();
    },
    _touchGesture : function(event) {

        if(event && event.scale && event.scale != 1) {

            this._trackball.zoomFactor(event.scale);
            
            this._updateMatrices();
            this._showCorrect();
        }
        
        event.preventDefault();
    },
    _mousedown : function(event) {
        this._active = true;
        
        var x = event.pageX - this._placementX();
        var y = event.pageY - this._placementY();
        event.relativeX = x;
        event.relativeY = y;
        for(var i = 0; i < this._eventHandlers.length; ++i) {
            this._eventHandlers[i].mousePressEvent(event);
        }
        this._showCorrect();
        event.preventDefault();
        
    },
    _placementX : function() {
        return dojo.position(this._canvas).x;
    },
    
    _placementY : function() {
        return dojo.position(this._canvas).y;
    },
    _mouseup : function(event) {
        this._active = false;
        var x = event.pageX - this._placementX();
        var y = event.pageY - this._placementY();
        event.relativeX = x;
        event.relativeY = y;
        for(var i = 0; i < this._eventHandlers.length; ++i) {
            this._eventHandlers[i].mouseReleaseEvent(event);
        }
        this._showCorrect();
        event.preventDefault();
    },

    _mousemove : function(event) {
        var x = event.pageX - this._placementX();
        var y = event.pageY - this._placementY();
        event.relativeX = x;
        event.relativeY = y;
        for(var i = 0; i < this._eventHandlers.length; ++i) {
            this._eventHandlers[i].mouseMoveEvent(event);
        }

        event.preventDefault();

    },

    _updateMatrices : function() {
        return;
        var viewer = this._modelLib.getValue(this._key);
        this._imageLoading = true;
        this._modelLib.updateElement(this._key, viewer);
    },

    _setAttrWidthHeight : function(node) {
        dojo.attr(node, "width", this._width + "px");
        dojo.attr(node, "height", this._height + "px");
    },

    _setWidthHeight : function(node) {
        dojo.style(node, "width", this._width + "px");
        dojo.style(node, "height", this._height + "px");
    },

    _startGL : function() {
        this._gl = WebGLUtils.setupWebGL(this._canvas);
        if(this._gl) {
            this._render_list_store = new renderlist.RenderListStore(this._gl);
            this._render_list_parser = new renderlist.RenderListParser();
            this._render_list_renderer = new renderlist.RenderListRenderer(this._gl);
            this._getRenderList();
        }
    },


    _getRenderList: function(){
        if( this._render_list_store === undefined ) {
            return;
        }

        dojo.xhrGet(
        {
                url: this._renderListURL + "?key=" + this._key + "&timestamp="+this._render_list_store.revision(),
            handleAs: "xml",
            timeout: 10000,
            load: dojo.hitch(this, function(data, ioargs) {
                this._render_list_parser.parse(this._render_list_store, data);
                this._updateMatrices();
                this._render();
                return data;
            })
        }
        );
    },

    _render  : function() {
        window.requestAnimFrame(dojo.hitch(this, function(){
            this._render();
        }));
        
        var viewer = this._modelLib.getElementValue(this._key);
        var view_coord_sys = {
            m_projection	: viewer.getElementValue("projection"),
            m_projection_inverse : mat4.inverse(mat4.create(viewer.getElementValue("projection"))),
            m_to_world		: mat4.inverse(mat4.create(viewer.getElementValue("modelview"))),
            m_from_world	: viewer.getElementValue("modelview")
        }
        this._render_list_renderer.render(this._render_list_store, view_coord_sys);
    },

    _loadImage : function() {
	return;
        if(this._updatingBeforeLoad || this._localMode || this._active) {
            return;
        }

        console.log("trying to load image");
        this._loadImageAgain = 0;
        this._loadImageStage2();
    },

    _loadImageStage2 : function() {
        this._DEBUG_imageLoadStart = (new Date()).getTime();
        this._imageLoading = true;
        this._showCorrect();
        //dojo.connect(this._img, "onload", this._onLoadFunction);
       // dojo.attr(this._img, "src", this._makeImgURL());
    },

    _loadComplete : function() {
        console.log("Time taken to load image: ", (new Date()).getTime()-this._DEBUG_imageLoadStart);
        this._imageLoading = false;

        if(this._loadImageAgain--) {

            this._loadImageStage2();
        } else {

        }
        this._showCorrect();
    },

    _makeImgURL : function() {
        return this._snapshotURL + "?width="+this._width+"&height="+this._height
        + "&timestamp=" + (new Date()).getTime() + "&key="+ this._key;
    },

    _update : function() {
        //if(this._localMode) return;

        this._showCorrect();
        this._loadImage();
    },

    _showCorrect: function() {
        if( this._localMode || (this._showRenderList && this._active) ) {// ||  (this._imageLoading && this._mouseOver )) {
            dojo.style(this._img, "z-index", "0");
            this._img.style.zIndex = "0";
        }
        else {
            dojo.style(this._img, "z-index", "2");
            this._img.style.zIndex = "2";
        }
    },

    _mouseMoveResize: function(event) {
        var x = event.pageX - this._placementX();
        var y = event.pageY - this._placementY();

        if(this._isResizing) {
            this.resize(x, y);
        }

    },

    _mouseDownResize: function(event) {
        if(event.button != 0) return;
        var x = event.pageX - this._placementX();
        var y = event.pageY - this._placementY();
        if(Math.max(Math.abs(this._width-x), Math.abs(this._height-y)) < 20) {
            this._isResizing = true;
        }
    },

    _mouseUpResize: function(event) {
        this._isResizing = false;
    }



})
