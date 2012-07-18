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

Compilation and running on Windows is definitively quite complicated, 
and it is not possible to guarantee 100% compatability with other software
you have installed.

### Dependencies

- CMake 2.8 from <http://www.cmake.org>
- Qt 4.8 from <http://qt.nokia.com/downloads> you need just the [Qt libraries](http://qt.nokia.com/downloads/windows-cpp-vs2010), but
  feel free to grab the larger SDK packages if you prefer them. 
- A precompiled archive of [Boost](http://www.boost.org), [GLEW](http://glew.sf.net) and [GLM](http://glm.g-truc.net/)
  from can be downloaded from our [Github](https://github.com/downloads/hetcomp/tinia/Tinia_3rdParty-18.07.12.zip) page.
  
### Generating a Visual Studio Solution file

Once the dependencies are installed, we invoke CMake to generate a Visual Studio
solution file that can be compiled either on the command line by `msbuild` or loaded
into the IDE. 

For the following example, we assume you have extracted the precompiled 
package to <tinia_3rdparty_location> and that Qt is installed by the normal installer (so that
it can be found either by registry keys or the `QTDIR` environment variable).

You can now invoke the Cmake-gui and point it to the folder where the Tinia
sources are installed. If you are using Visual Studio and intend to _modify_
the Tinia sources, we recommend letting the build directory be the same directory
as the source code (known as an  in-source-build).

Click `configure` and it will probably complain about not finding Boost, GLM and
Glew. This situation can be fixed by expanding the Tinia-section and setting
the variable Tinia_3RDPARTY_LOC to point to <tinia_3rdparty_location>.

Hit `configure` again, and you should hopefully find all dependencies.
It is now time to click `generate` which should make a `tinia.sln` file
you can open in Visual Studio.

**The generated `tinia.sln` file contain absolute paths on your computer
and is not suitable beeing checked into version control**.

### Compilation

Compilation should now be relatively straightforward. The `tinia.sln` file
produced by the above step can be loaded into Visual Studio and compiled using
the IDE, or you can open a Visual Studio Command prompt and invoke `msbuild tinia.sln`
which will build the libraries, unit tests, tutorials and examples.

### Running Tinia Application

Running the compiled programs can however be a bit troublesome.
You _must_ ensure that the following is in you path:
- `<tinia_3rdparty_location>\bin` 
- `QTDIR`\bin

It is possible you have installed other tools which include dlls from 
the package above in your path. However, they might very well be of
different and incompatible versions than the ones provided in the above 
packages leading to the so called Windows DLL-hell.

For invoking Tinia-application you can ensure that the above directories are
_first_ in your path, this should be acceptable for developement. 