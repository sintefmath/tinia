function TiniaApp( binary )
{
    this.m_id = binary;
    this.m_binary = binary;
}
TiniaApp.prototype =
{
binary:
    function() {
        return this.m_binary;
    }

};
// -----------------------------------------------------------------------------
function TiniaJob( id, pid, app, state )
{
    this.m_id = id;
    this.m_pid = pid;
    this.m_app = app;
    this.m_state = state;
}
TiniaJob.prototype =
{
id:
    function() {
        return this.m_id;
    },
pid:
    function() {
        return this.m_pid;
    },
app:
    function() {
        return this.m_app;
    },
state:
    function() {
        return this.m_state;
    }
};
// -----------------------------------------------------------------------------
function TiniaRenderingDevice( id )
{
    this.m_id = id;
    this.m_major = 0;
    this.m_minor = 0;
    this.m_renderer_string = "";
    this.m_version_string = "";
    this.m_vendor_string = "";
    this.m_error = "";
    
}
TiniaRenderingDevice.prototype =
{
id:                 function()  { return this.m_id; },
majorVersion:       function()  { return this.m_major; },
minorVersion:       function()  { return this.m_minor; },
setVersion:         function( major, minor ) { this.m_major = major; this.m_minor = minor; return this; },
rendererString:     function()  { return this.m_renderer_string; },
setRendererString:  function(a) { this.m_renderer_string=a; return this; },
versionString:      function()  { return this.m_version_string; },
setVersionString:   function(a) { this.m_version_string=a; return this; },
vendorString:       function()  { return this.m_vendor_string; },
setVendorString:    function(a) { this.m_vendor_string=a; return this; },
error:              function()  { return this.m_error; },
setError:           function(a) { this.m_error = a; return this; }

};

// -----------------------------------------------------------------------------
function TiniaPoke() {
    this.m_jobs = new Array;
    this.m_apps = new Array;
    this.m_rdevs = new Array;
    this.m_rdevs_error = "";
    
}
TiniaPoke.prototype =
{
clearRenderingDeviceList:  function()   { this.m_rdevs=new Array; this.m_rdevs_error = ""; return this; },
addRenderingDevice:        function(a)  { if(a instanceof TiniaRenderingDevice) { this.m_rdevs.push(a); } return this; },
renderingDevicesCount:     function()   { return this.m_rdevs.length; },
renderingDevice:           function(ix) { return this.m_rdevs[ix]; },
renderingDevicesError:     function()   { return this.m_rdevs_error; },
setRenderingDevicesError:  function(a)  { this.m_rdevs_error = a; return this; },

clearAppList:
    function() {
        this.m_apps = new Array;
        return this;
    },
addApp:
    function( app ) {
        if( app instanceof TiniaApp ) {
            this.m_apps.push( app );
        }
    },
appCount:
    function() {
        return this.m_apps.length;
    },
app:
    function( index ) {
        return this.m_apps[index];
    },

clearJobList:
    function() {
        this.m_jobs = new Array;
        return this;
    },
addJob:
    function( job ) {
        if( job instanceof TiniaJob ) {
            this.m_jobs.push( job );
        }
    },
jobCount:
    function() {
        return this.m_jobs.length;
    },
job:
    function( index ) {
        return this.m_jobs[ index ];
    },
jobById:
    function( id ) {
        var i;
        for(i=0; i<this.m_jobs.length; i++ ) {
            var job = this.m_jobs[i];
            if( job.id() == id ) {
                return job;
            }
        }
        return null;
    }
};




//var jsFile = "common.js";
//var tmp = '<script type="text/javascript" src="' + jsFile + '"></scr' + 'ipt>';
//window.console.log( "tmp: " + tmp );
//document.write(tmp);

function
init()
{
    tinia_poke = new TiniaPoke();

    periodic();
    if ( !isDemo() ) {
	listRenderingDevices( );
	listApplications( 0 );
    }
}



function
periodic()
{
    var i;
    update();
    setTimeout( periodic, 10000 );
}


function
update()
{
    listJobs();
    if ( !isDemo() ) {
	load();
    }
}

function dumpXML( req )
{
    window.console.log( "XML: " + req.responseText );
}


function isDemo() {
    return (window.location.pathname.indexOf("demoes") > -1);
}




function interact( id )
{
  window.open( '../job/' + id + "/sessionid/index.html", '_'+id ); // Don't know why, but using this, stuff written to the console log seems to disappear.
  // window.location = '../job/' + id + "/sessionid/index.html", '_'+id;
}




