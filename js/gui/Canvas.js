dojo.provide("gui.Canvas");

dojo.require("3rdparty.webgl-utils");
dojo.require("renderlist.RenderList");
dojo.require("renderlist.RenderListParser" );
dojo.require("renderlist.RenderListRenderer" );
dojo.require("dijit._Widget");
dojo.require("gui.TrackBallViewer");
dojo.require("3rdparty.glMatrix");

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
        this._policyLib = params.policyLib;
        this._snapshotURL = params.snapshotURL;

        this._localMode = params.localMode;

        this._urlHandler.updateParams({"key": this._key});
        this._onLoadFunction = dojo.hitch(this, this._loadComplete);
    },

    resize: function(w, h) {
        this._width = w;
        this._height = h;
        this._setWidthHeight(this.domNode);
        this._setAttrWidthHeight(this._img);
        this._setAttrWidthHeight(this._canvas);
        this._placeImage();
        this._trackball.setSize(w, h);

        this._urlHandler.updateParams({
            "width" : w,
            "height" : h
        });



        var viewer = this._policyLib.getValue(this._key);
        viewer.updateElement("Width", w);
        viewer.updateElement("Height", h);
        this._policyLib.updateElement(this._key, viewer);


    //this._update();

    },

    buildRendering: function() {
        this.domNode = dojo.create("div");
        dojo.style(this.domNode, "background", "black");
        dojo.style(this.domNode, "margin", "10px");


        this._setWidthHeight(this.domNode);
        this._canvas = dojo.create("canvas");
        dojo.style(this._canvas, "position", "relative");
        dojo.style(this._canvas, "float", "none");
        dojo.style(this._canvas, "display", "block");
        this._canvas.style.zIndex = "1";
        dojo.style(this._canvas, "z-index", "1");

        this._setAttrWidthHeight(this._canvas);
        
        if( window.WebGLRenderingContext ) {
            this.domNode.appendChild(this._canvas);
        }



        this._trackball = new gui.TrackBallViewer();
        this._trackball.setSize(this._width, this._height);


        this._img = dojo.create("img");
        dojo.attr(this._img, "src", "image/canvas_backdrop.jpg");
        dojo.attr(this._img, "alt", "Image from server");
        dojo.style(this._img, "position", "relative");
        dojo.style(this._img, "float", "none");
        dojo.style(this._img, "z-index", "2");
        this._placeImage();
        dojo.connect(this._img, "onload", dojo.hitch(this, this._loadComplete()));

        this._setAttrWidthHeight(this._img);

       this.domNode.appendChild(this._img);

        this._update();

        this._policyLib.addLocalListener(this._boundingboxKey, function(key, value) {
            this._trackball.setBoundingBox(value);
            this._updateMatrices();
            // We'll update the matrices twice, one time after the update is parsed as well
            this._shouldUpdate = true;
        }, this);
        
        
        this._policyLib.addLocalListener(this._resetViewKey, function(key, value) {
            this._trackball = new gui.TrackBallViewer();
            this._trackball.setSize(this._width, this._height);
            this._trackball.setBoundingBox(value);
            this._updateMatrices();
        });

        this._policyLib.addLocalListener(this._renderlistKey, function(key, value) {
            this._getRenderList();
        }, this);

        if(this._policyLib.hasKey(this._boundingboxKey)) {
            this._trackball.setBoundingBox(this._policyLib.getValue(this._boundingboxKey));
        }

        // We always want to update after the first parsing
        this._shouldUpdate = true;
        
        this._urlHandler.setURL(this._snapshotURL);
        dojo.subscribe("/policylib/updateParsed", dojo.hitch(this, function(params) {
            if(this._shouldUpdate) {
                this._updateMatrices();
            }
            this._shouldUpdate = false;
        }));

        dojo.subscribe("/policylib/updateSendPartialComplete", dojo.hitch(this, function(params) {
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
	this.domNode.addEventListener("touchstart",  dojo.hitch(this, this._mousedown));
	this._img.addEventListener("touchstart", dojo.hitch(this, this._mousedown));
        dojo.connect(document,"onmouseup", dojo.hitch(this, this._mouseup));
	this.on("touchend", dojo.hitch(this, this._mouseup));
        this.on("mousemove", dojo.hitch(this, this._mousemove));
	document.addEventListener("touchmove", dojo.hitch(this, this._mousemove));
        this.on("mouseover", dojo.hitch(this, function() {
            this._mouseOver = true;
        }));
        this.on("mouseout", dojo.hitch(this, function() {
            this._mouseOver = false;
        }));
  
        document.addEventListener("gesturestart", function(event) {console.log("hei");});
        document.addEventListener("gesturestart", dojo.hitch(this, this._touchGestureBegin));
        document.addEventListener("gesturechange", dojo.hitch(this, this._touchGesture));
        //this.on("gesturestart", dojo.hitch(this, this._touchGestureBegin));
        
        //this.on("gesturechange", dojo.hitch(this, this._touchGesture));

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

        var x = event.pageX - this._canvas.offsetLeft;
        var y = event.pageY - this._canvas.offsetTop;
	if(event.touches) {
            if(event.touches.length > 1) {
                return;
            }
	    this._trackball.rotationBegin(x, y);
	    this._active = true;
	} 
	else {
        switch(event.button) {
            case 0:
                if(event.ctrlKey) {
                    this._trackball.zoomBegin(x, y);
                    this._active = true;
                    break;
                }
                else {
                    this._trackball.rotationBegin(x, y);
                    this._active = true;
                    break;
                }
            case 1:
                this._trackball.zoomBegin(x, y);
                this._active = true;
                break;
        }
	}
        if(this._active) {
            this._updateMatrices();
            if(event.preventDefault) {
                event.preventDefault();
            }
            this._showCorrect();
        }

        event.preventDefault();
    },

    _mouseup : function(event) {
        if(!this._active) return;
        var x = event.pageX - this._canvas.offsetLeft;
        var y = event.pageY - this._canvas.offsetTop;

        this._trackball.rotationEnd(x, y);

        this._trackball.zoomEnd(x, y);

        this._updateMatrices();
        this._showCorrect();
        this._active = false;
    //	setTimeout(1000, dojo.hitch(this, this._showCorrect()));
    },

    _mousemove : function(event) {
	if(event.touches) {
            if(event.touches.length > 1) {
                return;
            }
	    event = event.touches[0];
	}
        var x = event.pageX - this._canvas.offsetLeft;
        var y = event.pageY - this._canvas.offsetTop;
        if(this._active) {
            this._trackball.mouseMove(x,y);
            this._updateMatrices();
            this._showCorrect();
        }



    },

    _updateMatrices : function() {
        this._trackball.updateMatrices();

        var viewer = this._policyLib.getValue(this._key);
        viewer.updateElement("Projection", this._trackball.getProjection());
        viewer.updateElement("Modelview", this._trackball.getModelView());


        this._imageLoading = true;

        this._policyLib.updateElement(this._key, viewer);
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

        this._render_list_renderer.render(this._render_list_store, this._trackball.getViewCoordSys());
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
        var x = event.pageX - this._canvas.offsetLeft;
        var y = event.pageY - this._canvas.offsetTop;

        if(this._isResizing) {
            this.resize(x, y);
        }

    },

    _mouseDownResize: function(event) {
        if(event.button != 0) return;
        var x = event.pageX - this._canvas.offsetLeft;
        var y = event.pageY - this._canvas.offsetTop;
        if(Math.max(Math.abs(this._width-x), Math.abs(this._height-y)) < 20) {
            this._isResizing = true;
        }
    },

    _mouseUpResize: function(event) {
        this._isResizing = false;
    }



})
