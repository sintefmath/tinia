

//var jsFile = "common.js";
//var tmp = '<script type="text/javascript" src="' + jsFile + '"></scr' + 'ipt>';
//window.console.log( "tmp: " + tmp );
//document.write(tmp);

function
init()
{
    periodic();
    listRenderingDevices( );
    listApplications( 0 );
}



function
periodic()
{
    update();
    setTimeout( periodic, 100000 );
}


function
update()
{
    listJobs();
    load();
}

function dumpXML( req )
{
    window.console.log( "XML: " + req.responseText );
}












function interact( id )
{
  window.open( '../job/' + id + "/sessionid/index.html", '_'+id ); // Don't know why, but using this, stuff written to the console log seems to disappear.
  // window.location = '../job/' + id + "/sessionid/index.html", '_'+id;
}




