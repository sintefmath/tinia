

function updateAppList()
{
    var jobid   = document.getElementById( 'jobid' );
    var jobexe  = document.getElementById( 'jobexe' );
    var jobargs = document.getElementById( 'jobargs' );
    var runjob  = document.getElementById( 'commit_job' );
    
    while( jobexe.childNodes.length ) {
        jobexe.removeChild( jobexe.childNodes[0] );
    }
    
    
    var count = tinia_poke.appCount();
    if( count ) {
        for( var i=0; i<count; i++ ) {
            var app = tinia_poke.app( i );
            var option = document.createElement( 'option' );
            option.innerHTML = app.binary();
            jobexe.appendChild( option );
        }
        jobid.disabled   = false;
        jobexe.disabled  = false;
        jobargs.disabled = false;
        runjob.disabled  = false;
    }
    else {
        var option = document.createElement( 'option' );
        option.innerHTML = "[none found]";
        jobexe.appendChild( option );
        jobid.disabled   = true;
        jobexe.disabled  = true;
        jobargs.disabled = true;
        runjob.disabled  = true;
    }
}

function generateJobId()
{
    for( var q=1; q<1000; q++ ) {
        var candidate = 'job_' + q;
        if( tinia_poke.jobById( candidate ) == null ) {
            var element = document.getElementById( 'jobid' );
            element.value = candidate;
            return;
        }
    }

}

function updateRenderingDevicesList()
{

    div = document.getElementById( 'rendering_devices' );
    while( div.childNodes.length ) {
        div.removeChild( div.childNodes[0] );
    }

    var count = tinia_poke.renderingDevicesCount();
    if( count < 1 ) {
        var error = tinia_poke.renderingDevicesError();
        if( error ) {
            error = ' (why: ' + error + ')';
        }
        div.innerHTML = '<I>No available rendering devices'+error+'.</I>';
        return;
    }

    var table, row, cell;
    table = document.createElement( "table" );
    table.setAttribute( 'class', 'my' );
    div.appendChild( table );
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
    
    var select = document.getElementById( 'jobgldev' );
    while( select.childNodes.length ) {
        select.removeChild( select.childNodes[0] );
    }

    for( var i=0; i<count; i++ ) {
        var device = tinia_poke.renderingDevice(i);
        
        row = document.createElement( "tr" );
        row.setAttribute( 'class', 'my' );
        table.appendChild( row );
        cell = document.createElement( "td" );
        cell.setAttribute( 'class', 'my' );
        cell.innerHTML = device.id();
        row.appendChild( cell );

        if( device.error() ) {
            cell = document.createElement( "td" );
            cell.setAttribute( 'class', 'mo' );
            cell.setAttribute( 'colspan', 4 );
            cell.innerHTML = '<I>Unavailable ('+device.error()+')</I>';
            row.appendChild( cell );
        }
        else {
            var option = document.createElement( 'option' );
            option.innerHTML = device.rendererString() + ' ('+device.id()+')';
            option.setAttribute( 'value', device.id() );
            select.appendChild( option );
            
        
            cell = document.createElement( "td" );
            cell.setAttribute( 'class', 'my' );
            cell.innerHTML = device.majorVersion()+'.'+device.minorVersion();
            row.appendChild( cell );
            cell = document.createElement( "td" );
            cell.setAttribute( 'class', 'my' );
            cell.innerHTML = device.rendererString();
            row.appendChild( cell );
            cell = document.createElement( "td" );
            cell.setAttribute( 'class', 'my' );
            cell.innerHTML = device.versionString();
            row.appendChild( cell );
            cell = document.createElement( "td" );
            cell.setAttribute( 'class', 'my' );
            cell.innerHTML = device.vendorString();
            row.appendChild( cell );
        }
    }

    if( select.childNodes.length == 0 ) {
        var option = document.createElement( 'option' );
        option.innerHTML = "None";
        option.setAttribute( 'value', 'none' );
        select.appendChild( option );
    }

}


function updateJobList_()
{
    var div = document.getElementById( 'joblist' );

    while( div.childNodes.length ) {
        div.removeChild( div.childNodes[0] );
    }

    var count = tinia_poke.jobCount();
    if( count ) {
        var table, row, cell, button;
    
        table = document.createElement( 'table' );
        table.setAttribute( 'class', 'my' );
        div.appendChild( table );
        
        row = document.createElement( 'tr' );
        row.setAttribute( 'class', 'my' );
        table.appendChild( row );

        cell = document.createElement( 'th' );
        cell.setAttribute( 'class', 'my' );
        cell.innerHTML = "Job id";
        row.appendChild( cell );

        cell = document.createElement( 'th' );
        cell.setAttribute( 'class', 'my' );
        cell.innerHTML = "Pid";
        row.appendChild( cell );

        cell = document.createElement( 'th' );
        cell.setAttribute( 'class', 'my' );
        cell.innerHTML = "Application";
        row.appendChild( cell );

        cell = document.createElement( 'th' );
        cell.setAttribute( 'class', 'my' );
        cell.innerHTML = "State";
        row.appendChild( cell );
    
        for(var i=0; i<count; i++) {
            var job = tinia_poke.job( i );

            row = document.createElement( 'tr' );
            row.setAttribute( 'class', 'my' );
            table.appendChild( row );

            cell = document.createElement( 'td' );
            cell.setAttribute( 'class', 'my' );
            cell.innerHTML = job.id();
            row.appendChild( cell );

            cell = document.createElement( 'td' );
            cell.setAttribute( 'class', 'my' );
            cell.innerHTML = job.pid();
            row.appendChild( cell );

            cell = document.createElement( 'td' );
            cell.setAttribute( 'class', 'my' );
            cell.innerHTML = job.app();
            row.appendChild( cell );

            cell = document.createElement( 'td' );
            cell.setAttribute( 'class', 'my' );
            cell.innerHTML = job.state();
            row.appendChild( cell );
            
            cell = document.createElement( 'td' );
            cell.setAttribute( 'class', 'my' );
            row.appendChild( cell );

            if( (job.state() == "TERMINATED_SUCCESSFULLY") ||
                (job.state() == "TERMINATED_UNSUCCESSFULLY") )
            {
                button = document.createElement( "button" );
                button.innerHTML = "Wipe";
                button.setAttribute( 'onclick', 'wipe( "'+job.id()+'");' );
                button.setAttribute( 'title', 'Wipe this job from list of managed jobs. Not allowed for running jobs.' );
                cell.appendChild( button );
            }
            else {
                button = document.createElement( "button" );
                button.innerHTML = "Suicide";
                button.setAttribute( 'onclick', 'killJob( "'+job.id()+'", false );' );
                button.setAttribute( 'title', 'Cleanly kill job by sending a messsage to job suggesting a suicide.' );
                cell.appendChild( button );

                button = document.createElement( "button" );
                button.innerHTML = "Kill";
                button.setAttribute( 'title', 'Brutally kill job by sending it a SIGTERM signal. Try requesting a suicide first.' );
                button.setAttribute( 'onclick', 'killJob( "'+job.id()+'", true );' );
                cell.appendChild( button );
            
                button = document.createElement( "button" );
                button.innerHTML = "Interact";            
                button.setAttribute( 'onclick', 'interact( "' + job.id() + '");' );
                button.setAttribute( 'title', 'Interact with this job. Makes sense only for running jobs.' );
                cell.appendChild( button );
            }
        }
    }
    else {
        div.innerHTML = "<I>No jobs running.</I>";
    }    
}



