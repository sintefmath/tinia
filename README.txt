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
