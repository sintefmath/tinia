function addDemoJob( application, args )
{
    // args have to be the format "<arg>argument</arg>"
    // If multiple arguments: "<arg>arg_1</arg><arg>arg_2</arg>...<arg>arg_n</arg>

    window.console.log("Starting addDemoJob with application " + application);
    var req = new XMLHttpRequest();
    req.open( "POST", "../master/rpc.xml?xml&foo=bar" );
    req.setRequestHeader( "Content-Type", "text/xml" );
    
    req.onreadystatechange = function() {
        if( this.readyState == 4 && this.status == 200 ) {
            update( this );
            window.console.log( "rpcMaster: Got: " + this.responseText );
        }
    };

    var date = new Date();
    var job_id = application + "_" + date.getTime();
    
    window.console.log( "rpcMaster: Sending: <here should there be xml...>"  );
    req.send( '<?xml version="1.0"?>' +
              '<query xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="http://cloudviz.sintef.no/trell/1.0">'+
              '<addJob>' +
              '<job>' + job_id + '</job>' +
              '<application>' + application + '</application>' + 
	      args +
              '<renderingDeviceId>:0.0</renderingDeviceId>' +
              '</addJob>' +
              '</query>' );
    
    // Open job:
    job_url = "../job/" + job_id + "/sessionid/index.html";
    window.open(job_url, "_blank");
}


function demo_startFRView()
{
    addDemoJob("frview_cloud_job", '<arg>/usr/local/var/trell/apps/BC0407/BC0407.EGRID</arg>');
}

function demo_startClock()
{
    addDemoJob("clock_cloud_job", "");
}

function demo_startAP()
{
    addDemoJob("jpc_cloud_job", "");
}

function demo_startTwoCanvas()
{
    addDemoJob("two_cloud_job", "");
}
