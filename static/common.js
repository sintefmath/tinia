function updateLoad( req )
{
    var doc, div, table, row, cell, k, j, i;

    doc = req.responseXML;
    if( doc == null ) {
        return;
    }

    table = document.createElement( "table" );
    row = document.createElement( "tr" );
    table.appendChild( row );


    for( k = doc.firstChild; k; k=k.nextSibling ) {
        if( k.nodeName == 'reply' ) {
            for( j=k.firstChild; j; j=j.nextSibling) {
                if( j.nodeName == 'serverLoad' ) {
                    for( i=j.firstChild; i; i=i.nextSibling ) {
                        cell = document.createElement( "td" );
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




function dumpXML( req )
{
    window.console.log( "XML: " + req.responseText );
}




function rpcMaster( xml, dumper )
{
    var req = new XMLHttpRequest();
    req.open( "POST", "../master/rpc.xml?xml&foo=bar" );
    req.setRequestHeader( "Content-Type", "text/xml" );

    req.onreadystatechange = function() {
      if( this.readyState == 4 && this.status == 200 ) {
	dumper( this );
	window.console.log( "rpcMaster: Got: " + this.responseText );
      }
    };

    window.console.log( "rpcMaster: Sending: " + xml );
    req.send( '<?xml version="1.0"?>' +
              '<query xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="http://cloudviz.sintef.no/trell/1.0">'+
               xml +
              '</query>' );
}




function load()
{
  rpcMaster( '<getServerLoad/>', updateLoad );
}




function xmlEscape(xml)
{
  var xml2 = xml.replace(/</g, "&lt;");
  xml2 = xml2.replace(/>/g, "&gt;");
  return xml2;
}




