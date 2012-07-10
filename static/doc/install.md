Compilation and installation {#sec_compilation}
============================

Compilation is done by using cmake and to compile the package you need the following
Ubuntu packages (with dependencies) from Ubuntu 12.04:
    - libxml2-dev
    - glew1.6-dev
    - libglm-dev
    - libqt4-dev
    - libapr1-dev
    - cmake
    - libboost-all-dev (For unit tests).
    - apache2-dev (For server module).

     Once these are installed a simple `$ cmake . ` `$ make `.

     Should be enough to compile it. You can optionally issue
     ` $ make package` Which will build a debian package which installs into
     /usr/lib and /usr/include/model . This package can be installed on your
     system with the following command:

     `$ sudo dpkg -i ExposedModel<version>.deb \endverbatim`
