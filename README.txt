ON LINUX
To compile, simply issue the following commands:
   mkdir build
   cd build
   cmake ..
   make 

To validate, see if examples/simplejob/simple_qt_job starts.

To install:
   make package
   sudo dpkg -i tinia-0.0.0-Linux.deb

(This might take a while, as it also downloads some javascript components)

ON WINDOWS:
To use this software you will need the following 3rd party libraries:
Boost, version 1.46 or greater.
GLM, version 0.92 or greater.
GLEW, version 1.7.0 or greater.

Qt, version 1.47.0 or greater.

You can find a .zip with all but Qt at the following location
\\sintef.no\ikt\prosjekt\9011-Math\HETEROCOMP\thirdparty\Tinia

Unpack these to a folder and create an environment variable called
"Tinia_3rdParty" pointing to it.

Install Qt to where you want it, but make sure there is an 
environment variable named "QtDir" pointing to the correct location.

With that done you should be able to simply open the Tinia solution
(Tinia.sln) in Visual Studio 2010 and compile the library.

The resulting .lib files will be located in "lib/(Debug|Release)"
under the Tinia base folder.