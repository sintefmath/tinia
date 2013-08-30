function updateLoad( req )
{
    var doc, div, table, row, cell, k, j, i;

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

    div = document.getElementById( 'load' );
    if( div == null ) {
      window.console.log( "Could not find 'load'-tag in html" );
        return;
    }
    while( div.childNodes.length ) {
        div.removeChild( div.childNodes[0] );
    }
    div.appendChild( table );
}

function
listRenderingDevicesReplyHandler( req )
{
    var doc, errors;

    doc = req.responseXML;
    if( doc == null ) {
        return;
    }

    div = document.getElementById( 'rendering_devices' );
    if( div == null ) {
        return;
    }
    while( div.childNodes.length ) {
        div.removeChild( div.childNodes[0] );
    }

    var error = doc.getElementsByTagName( 'error' );
    if( error && error.length > 0 ) {
        div.innerHTML = "None (" + error.item(0).textContent + ")";
        return;
    }

    var devices = doc.getElementsByTagName( 'renderingDevice' );
    if( devices.length == 0 ) {
        div.innerHTML = "None";
        return;
    }

    var table, row, cell;
    table = document.createElement( "table" );
    table.setAttribute( 'class', 'my' );
    row = document.createElement( "tr" );
    row.setAttribute( 'class', 'my' );
    table.appendChild( row );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "Id";
    row.appendChild( cell );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "OpenGL";
    row.appendChild( cell );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "Renderer";
    row.appendChild( cell );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "Version string";
    row.appendChild( cell );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "Vendor";
    row.appendChild( cell );
    
    for( var k = 0; k < devices.length; k++ ) {
        var device, nodelist;
        var device = devices.item( k );

        var device_id = device.getAttribute( 'number' );        
        var opengl_renderer = "";
        var opengl_version = "";
        var opengl_vendor = "";
        var opengl_major = 0;
        var opengl_minor = 0;

        var opengl = device.getElementsByTagName( 'opengl' );
        if( opengl.length > 0 ) {
            opengl_major = opengl.item(0).getAttribute( 'major' );

            nodelist = opengl.item(0).getElementsByTagName( 'vendor' );
            if( nodelist.length > 0 ) {
                opengl_vendor = nodelist.item(0).textContent;
            }

            nodelist = opengl.item(0).getElementsByTagName( 'version' );
            if( nodelist.length > 0 ) {
                opengl_version = nodelist.item(0).textContent;
            }

            nodelist = opengl.item(0).getElementsByTagName( 'renderer' );
            if( nodelist.length > 0 ) {
                opengl_renderer = nodelist.item(0).textContent;
            }
        }


        row = document.createElement( "tr" );
        row.setAttribute( 'class', 'my' );
        table.appendChild( row );
        cell = document.createElement( "td" );
        cell.setAttribute( 'class', 'my' );
        cell.innerHTML = device_id;
        row.appendChild( cell );
        cell = document.createElement( "td" );
        cell.setAttribute( 'class', 'my' );
        cell.innerHTML = opengl_major + '.' + opengl_minor;
        row.appendChild( cell );
        cell = document.createElement( "td" );
        cell.setAttribute( 'class', 'my' );
        cell.innerHTML = opengl_renderer;
        row.appendChild( cell );
        cell = document.createElement( "td" );
        cell.setAttribute( 'class', 'my' );
        cell.innerHTML = opengl_version;
        row.appendChild( cell );
        cell = document.createElement( "td" );
        cell.setAttribute( 'class', 'my' );
        cell.innerHTML = opengl_vendor;
        row.appendChild( cell );
    }

    
    div.appendChild( table );
    


//    doc.getElementsByTagName( 'rende

    for( k = doc.firstChild; k; k=k.nextSibling ) {
        if( k.nodeName == 'reply' ) {
            for( j=k.firstChild; j; j=j.nextSibling) {
                if( j.nodeName == 'renderingDevices' ) {
                    for( i=j.firstChild; i; i=i.nextSibling ) {
                        if( i.nodeName == 'renderingDevice' ) {
                            var device_id = i.getAttribute( 'number' );
                            var opengl_vendor = "";
                            var opengl_renderer = "";
                            var opengl_version = "";


                            for( l = i.firstChild; l; l=l.nextSibling ) {
                            
                                if( l.nodeName == 'opengl' ) {
                                    
                                    for( m=l.firstChild; m; m=m.nextSibling ) {
                                        if( m.nodeName == 'vendor' ) {
                                            opengl_version = m.childNodes[0].nodeValue;
                                        }
                                        else if( m.nodeName == 'version' ) {
                                            opengl_version = m.childNodes[0].nodeValue;
                                        }
                                        else if( m.nodeName == 'renderer' ) {
                                            opengl_renderer = m.childNodes[0].nodeValue;
                                        }
                                    }
                                }

                            }

                        }
                    }
                }
            }
        }
    }
}
    

function
updateJobList( req )
{
    var doc, div, table, row, cell, reply, jobList, jobInfo, item, spec,
    stdout, stderr, suicide, kill, wipe, interact, t;
    var id_list = new Array;

    doc = req.responseXML;
    if( doc == null ) {
        window.console.log( "doc==null" );
        return;
    }

    table = document.createElement( "table" );
    table.setAttribute( 'class', 'my' );
    row = document.createElement( "tr" );
    row.setAttribute( 'class', 'my' );
    table.appendChild( row );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "Id";
    row.appendChild( cell );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "Pid";
    row.appendChild( cell );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "Executable";
    row.appendChild( cell );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "stdout";
    row.appendChild( cell );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "stderr";
    row.appendChild( cell );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "State";
    row.appendChild( cell );
    cell = document.createElement( "th" );
    cell.setAttribute( 'class', 'my' );
    cell.innerHTML = "Manage";
    row.appendChild( cell );

    for( reply = doc.firstChild; reply; reply = reply.nextSibling ) {
        if( reply.nodeName == 'reply' ) {
            for( jobList = reply.firstChild; jobList; jobList = jobList.nextSibling ) {
                if( jobList.nodeName == 'jobList' ) {
                    for( jobInfo=jobList.firstChild; jobInfo; jobInfo=jobInfo.nextSibling ) {
                        if( jobInfo.nodeName == 'jobInfo' ) {
                            spec = {};
                            for( item=jobInfo.firstChild; item; item=item.nextSibling ) {
                                if( item.nodeName == 'job' ) {
                                    spec['id'] = item.childNodes[0].nodeValue;
                                    id_list.push( spec['id'] );
                                }
                                else if( item.nodeName == 'pid' ) {
                                    spec['pid'] = item.childNodes[0].nodeValue;
                                }
                                else if( item.nodeName == 'application' ) {
                                    spec['exe'] = item.childNodes[0].nodeValue;
                                }
                                else if( item.nodeName == 'state' ) {
                                    spec['state'] = item.childNodes[0].nodeValue;
                                }
                            }

                            row = document.createElement( "tr" );
                            row.setAttribute( 'class', 'my' );
                            table.appendChild( row );
                            cell = document.createElement( "td" );
                            cell.setAttribute( 'class', 'my' );
                            cell.innerHTML = spec['id'];
                            row.appendChild( cell );
                            cell = document.createElement( "td" );
                            cell.setAttribute( 'class', 'my' );
                            cell.innerHTML = spec['pid'];
                            row.appendChild( cell );
                            cell = document.createElement( "td" );
                            cell.setAttribute( 'class', 'my' );
                            cell.innerHTML = spec['exe'];
                            row.appendChild( cell );
                            cell = document.createElement( "td" );
                            cell.setAttribute( 'class', 'my' );
                            cell.innerHTML = spec['stdout'];
                            row.appendChild( cell );
                            cell = document.createElement( "td" );
                            cell.setAttribute( 'class', 'my' );
                            cell.innerHTML = spec['stderr'];
                            row.appendChild( cell );
                            cell = document.createElement( "td" );
                            cell.setAttribute( 'class', 'my' );
                            cell.innerHTML = spec['state'];
                            row.appendChild( cell );
                            cell = document.createElement( "td" );
                            stdout = document.createElement( "button" );
                            cell.setAttribute( 'class', 'my' );
                            stdout.innerHTML = "stdout";
                            cell.appendChild( stdout );
                            stderr = document.createElement( "button" );
                            cell.setAttribute( 'class', 'my' );
                            stderr.innerHTML = "stderr";
                            cell.appendChild( stderr );
                            suicide = document.createElement( "button" );
                            suicide.setAttribute( 'onclick', 'killJob( "'+spec['id']+'", false );' );
                            suicide.setAttribute( 'title', 'Cleanly kill job by sending a messsage to job suggesting a suicide.' );
                            suicide.innerHTML = "Suicide";
                            cell.appendChild( suicide );
                            kill = document.createElement( "button" );
                            kill.setAttribute( 'title', 'Brutally kill job by sending it a SIGTERM signal. Try requesting a suicide first.' );
                            kill.setAttribute( 'onclick', 'killJob( "'+spec['id']+'", true );' );
                            kill.innerHTML = "Kill";
                            cell.appendChild( kill );
                            wipe = document.createElement( "button" );
                            wipe.setAttribute( 'title', 'Wipe this job from list of managed jobs. Not allowed for running jobs.' );
                            wipe.setAttribute( 'onclick', 'wipe( "'+spec['id']+'");' );
                            wipe.innerHTML = "Wipe";
                            cell.appendChild( wipe );
                            interact = document.createElement( "button" );
                            interact.setAttribute( 'title', 'Interact with this job. Makes sense only for running jobs.' );
                            interact.setAttribute( 'onclick', 'interact( "' + spec['id'] + '");' );
                            interact.innerHTML = "Interact";
                            cell.appendChild( interact );
                            row.appendChild( cell );
                        }
                    }
                }
            }
        }
    }

    div = document.getElementById( 'joblist' );
    if( div == null ) {
        return;
    }
    while( div.childNodes.length ) {
        div.removeChild( div.childNodes[0] );
    }
    div.appendChild( table );

    // If jobid is nothing, try to create a new jobid.
    t = document.getElementById( 'jobid' );
    if( (t != null) && (t.value.length == 0) ) {
        for( var q=1; q<1000; q++) {
            var candidate = 'job_'+q;
            if( id_list.indexOf(candidate) == -1 ) {
                t.value = candidate;
                break;
            }
        }
    }

}

