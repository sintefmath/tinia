function updateLoad( req )
{
    var doc, div, table, row, cell, k, j, i;

    div = document.getElementById( 'load' );
    while( div.childNodes.length ) {
        div.removeChild( div.childNodes[0] );
    }

    if( req.status != 200 ) {
        div.innerHTML = "<I>Unable to contact master job (HTTP error " + req.status + ")</I>";
        return;
    }

    doc = req.responseXML;
    if( doc == null ) {
        return;
    }

    table = document.createElement( "table" );
    table.setAttribute( 'class', 'my' );
    row = document.createElement( "tr" );
    row.setAttribute( 'class', 'my' );
    table.appendChild( row );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "1min";
    row.appendChild( cell );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "5min";
    row.appendChild( cell );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "15min";
    row.appendChild( cell );


    row = document.createElement( "tr" );
    row.setAttribute( 'class', 'my' );
    table.appendChild( row );


    for( k = doc.firstChild; k; k=k.nextSibling ) {
        if( k.nodeName == 'reply' ) {
            for( j=k.firstChild; j; j=j.nextSibling) {
                if( j.nodeName == 'serverLoad' ) {
                    for( i=j.firstChild; i; i=i.nextSibling ) {
                        cell = document.createElement( "td" );
                        cell.setAttribute( 'class', 'my' );
                        cell.innerHTML = i.childNodes[0].nodeValue;
                        row.appendChild( cell );
                    }
                }
            }
        }
    }
    div.appendChild( table );
}

function
listApplicationsReplyHandler( req )
{
    tinia_poke.clearAppList();

    doc = req.responseXML;
    if( doc == null ) {
        return;
    }

    // update tinia_poke
    var apps = doc.getElementsByTagName( 'application' );
    for( var k = 0; k < apps.length; k++ ) {
        var app = apps.item( k );
        var binary = app.getAttribute( 'binary' );
        if( binary ) {
            tinia_poke.addApp( new TiniaApp( binary ) );
        }
    }

    // update HTML
    updateAppList();
}


function
listRenderingDevicesReplyHandler( req )
{
    tinia_poke.clearRenderingDeviceList();

    if( req.status != 200 ) {
        tinia_poke.setRenderingDevicesError( "HTTP error " + req.status );
        updateRenderingDevicesList();
        return;
    }

    var doc = req.responseXML;
    if( doc == null ) {
        return;
    }

    var error = doc.getElementsByTagName( 'error' );
    if( error && error.length > 0 ) {
        tinia_poke.setRenderingDevicesError( error.item(0).textContent );
        updateRenderingDevicesList();
        return;
    }

    var devices = doc.getElementsByTagName( 'renderingDevice' );
    for( var k = 0; k < devices.length; k++ ) {
        var device, nodelist;
        var device = devices.item( k );
        var device_id = device.getAttribute( 'number' );        

        var dev = new TiniaRenderingDevice( device_id );
        tinia_poke.addRenderingDevice( dev );


        var opengl = device.getElementsByTagName( 'opengl' );
        if( opengl.length > 0 ) {
            dev.setVersion( opengl.item(0).getAttribute( 'major' ),
                            opengl.item(0).getAttribute( 'minor' ) );
            dev.setRendererString( opengl.item(0).getElementsByTagName( 'renderer' ).item(0).textContent );
            dev.setVersionString( opengl.item(0).getElementsByTagName( 'version' ).item(0).textContent );
            dev.setVendorString( opengl.item(0).getElementsByTagName( 'vendor' ).item(0).textContent );
        }
    }
    updateRenderingDevicesList();
}
    

function
updateJobList( req )
{
    var doc = req.responseXML;
    if( doc == null ) {
        window.console.log( "doc==null" );
        return;
    }

    tinia_poke.clearJobList();
    var jobs = doc.getElementsByTagName( 'jobInfo' );
    for( var k=0; k<jobs.length; k++ ) {
        var job = jobs.item( k );
        var job_id = job.getElementsByTagName( 'job' ).item(0).textContent;
        var job_pid = job.getElementsByTagName( 'pid' ).item(0).textContent;
        var job_app = job.getElementsByTagName( 'application' ).item(0).textContent;
        var job_state = job.getElementsByTagName( 'state' ).item(0).textContent;
        // <allowed>..
        tinia_poke.addJob( new TiniaJob( job_id, job_pid, job_app, job_state ) ); 
    }
    updateJobList_();
}

