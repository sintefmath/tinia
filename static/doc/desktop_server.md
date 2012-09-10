The integrated server{#sec_desktop_server}
===================================
Each Tinia desktop program comes with the possibility to run a server from the
program itself. The user may start up the ordinary Tinia desktop program, then
from the program start the server and iteract with the program through a web
browser (either locally on the same machine or remotely).

Currently this feature is for testing and debugging purposes only.

Compilation
----------------
In order to compile the integrated server, [Libxml2](http://www.xmlsoft.org/)
needs to be available. If you installed Tinia following the
[compilation directions](@ref sec_compilation)
for Ubuntu or Windows, you already have this installed.

Starting the server
----------------
Start up any Tinia desktop program (e.g. the program from
[Tutorial1](@ref tut_tutorial1). In the upper left corner, there is a button
called "Run server (http://localhost:8080/index.html)". Click this button once
to start the server. Once the server is up, you may interact with the program
by pointing your web browser to <http:://localhost:8080/index.html>. You may
also interact with the server from another machine.


Stopping the server
-------------------
To stop the dekstop server, simply click on the button marked
"Run server (http://localhost:8080/index.html)" once more.


