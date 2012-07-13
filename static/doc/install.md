Compilation and installation {#sec_compilation}
============================

Compilation is done by using Cmake on both Linux and Windows.

The supported software platforms are Ubuntu Linux 12.04 LTS and Windows 7 with
Visual Studio 2010. 

Other fairly recent *NIXes will probably work, as long as they have a
fairly recent C++ compiler supporting C++11.

Compilation on Ubuntu 12.04
---------------------------
The supported Linux platform is Ubuntu 12.04 LTS, using GCC 4.6 utilizing 
dependencies from the official repositories.

### Dependencies 

Ubuntu packages (with dependencies) from Ubuntu 12.04:

- libxml2-dev
- glew1.6-dev
- libglm-dev
- libqt4-dev
- libapr1-dev
- cmake
- libboost-all-dev
- apache2-dev (For server module).
- build-essential

Once these are installed a simple 

    $ cmake . 
    $ make 

Should be enough to compile it. You can optionally issue
` $ make package` Which will build a debian package which installs into
/usr/lib and /usr/include/model . This package can be installed on your
system with the following command:

`$ sudo dpkg -i tinia-0.1.0.deb`

Compilation on Windows 7 with Visual Studio 2010
------------------------------------------------
To compile on Windows it is required to download and install several
thirdparty packages. 

### Dependencies

- CMake 2.8 from <http://www.cmake.org>
- Qt 4.8 from <http://qt.nokia.com/downloads> you need just the [Qt libraries](http://qt.nokia.com/downloads/windows-cpp-vs2010), but
  feel free to grab the larger SDK packages if you prefer them. 
- A precompiled archive of [Boost](http://www.boost.org), [GLEW](http://glew.sf.net) and [GLM](http://glm.g-truc.net/)
  from can be downloaded from our [Github](https://github.com/downloads/hetcomp/tinia/Tinia_3rdParty-25.04.12.zip) page.
  Extract the archive to a folder and make sure that the environment varible TINIA_3RDPARTY points
  to this folder.

### Compilation

Once the dependencies are installed, we invoke CMake to generate a Visual Studio
solution file that can be compiled either on the command line by `msbuild` or loaded
into the IDE. For the following example using the command line 
we assume that CMake is installed in your path and that Qt is installed by the normal installer (so that
it can be found either by registry keys or the `QTDIR` environment variable.

You can now invoke the Cmake-gui and point it to the folder where the Tinia
sources are installed. Click `generate` and you should hopefully find all 
dependencies.




