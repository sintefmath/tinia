dojo.declare("gui.ImageReceiver", null, {
    constructor : function(imageElement, url, type) {
        this._imageElement = imageElement;
        this._url = url;
        if(type === undefined) {
            this._type = "png";
        }
    },
    
    getImage: function(xml, url, type) {
        if(url === undefined) {
            url = this._url;
        }
        if(type === undefined) {
            type = this._type;
        }
        dojo.rawXhrPost({
            url: this._url,
            postData : xml,
            headers: {"Content-Type": "text/xml"},
            handleAs: "text",
            preventCache: true,
            load : dojo.hitch(this, function(response, ioArgs) {
                var img = this._imageElement;  
                img.src = "data:image/"+type+";charset=ASCII-US;base64," + response;                
                return response;
            }),
            error: dojo.hitch(this, function(response, ioArgs) {
                return response;
            })
            
        });
    }
});

