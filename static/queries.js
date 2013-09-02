function rpcMaster( query_xml, reply_handler )
{
    var req = new XMLHttpRequest();
    req.open( "POST", "../master/rpc.xml?xml&foo=bar" );
    req.setRequestHeader( "Content-Type", "text/xml" );

    req.onreadystatechange = function() {
        if( this.readyState == 4 /* && this.status == 200*/ ) {
	        reply_handler( this );
            //window.console.log( "rpcMaster: Got: " + this.responseText );
        }
    };
    //window.console.log( "rpcMaster: Sending: " + query_xml );
    req.send( '<?xml version="1.0"?>' +
              '<query xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"' +
              ' xmlns="http://cloudviz.sintef.no/trell/1.0">'+
              query_xml +
              '</query>' );
}

function load()
{
  rpcMaster( '<getServerLoad/>', updateLoad );
}

function
pingMaster()
{
    rpcMaster( '<ping/>',
               dumpXML );
}

function
addJob( job, application )
{
    rpcMaster( '<addJob>'+
               '  <job>'+job+'</job>'+
               '  <application>'+application+'<application/>' +
               '</addJob>',
               update );
}

function listJobs()
{
    rpcMaster( '<getJobList/>',
               updateJobList );
}

function
restartMaster()
{
    var req = new XMLHttpRequest();
    req.open( "GET", "../mod/rpc.xml?action=restart_master" )
    req.onreadystatechange = function() {
        window.console.log( "Got reply " );
        if( this.readyState == 4 && this.status == 200 ) {
            window.console.log( "master restarted" );
            setTimeout( listRenderingDevices, 1000 );
            setTimeout( listApplications, 1000 );
        }
    };
    req.send();
}


function killJob( job, force )
{
    rpcMaster( '<killJob>'+
               '  <job>'+job+'</job>'+
               '  <force>'+force+'</force>'+
               '</killJob>',
               listJobs );
}

function
wipe( job )
{
    rpcMaster( '<wipeJob>'+
               '  <job>'+job+'</job>'+
               '</wipeJob>',
               listJobs );
}

function
listRenderingDevices()
{
    rpcMaster( '<listRenderingDevices/>',
               listRenderingDevicesReplyHandler );
}

function
listApplications( timestamp )
{
    if( !timestamp ) {
        timestamp = 0;
    }

    rpcMaster( '<listApplications>' +
               '  <timestamp>'+timestamp+'</timestamp>' +
               '</listApplications>',
               listApplicationsReplyHandler );
}


function guiAddJob()
{
    var job, application, args, t, q, xml;

    t = document.getElementById( 'jobid' );
    if( t == null ) {
        return;
    }
    job = t.value;
    t.value = "";

    t = document.getElementById( 'jobexe' );
    if( t == null ) {
        return;
    }
    application = t.value;

    t = document.getElementById( 'jobargs' );
    if( t==null) {
        return;
    }
    args = t.value.match(/[^"\s]+|"(?:\\"|[^"])+"/g);

    if( !job ) {
        alert( "Empty job name" );
        return;
    }
    if( !application ) {
        alert( "Empty job executable" );
        return;
    }

    xml = '<addJob>\n'
        + '  <job>'+job+'</job>\n'
        + '  <application>'+application+'</application>\n'
    for( t in args ) {
        xml = xml
            + '  <arg>'
            + args[t].replace( /^"(.*)"$/, "$1" )     // Strip enclosing quotation marks, if exists.
                     .replace( /\&/g, '&amp;' )       // & -> &amp;
                     .replace( /\\"/g, '&quot;' )     // \" -> &quot;
                     .replace( /</g, '&lt;' )         // < -> &lt;
                     .replace( />/g, '&gt;' )         // > -> &gt;
            + '</arg>\n';
    }
    xml = xml
        + '</addJob>\n';
    
    rpcMaster( xml, listJobs );
    
}

