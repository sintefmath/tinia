Tinia Mainpage {#mainpage}
==============

Tinia is a C++ platform designed to make it easy creating applications that can
run both on desktops and on servers, potentially in the cloud. It is primarily
focused on applications doing rendering with OpenGL and compute intensive
programs using CUDA/OpenCL, but it can be used for most any application area.

The only difference between a cloud application and a desktop application is
just a few lines of code. This means the applications can be developed with all
the usual debugging tools available on the desktop, making sure the program is
running as it should before deploying on your servers.

See [the compilation instructions](@ref sec_compilation) for details on how to
compile and install the framework.

Tina Modules
------------

The Tinia Framework is split into several components, each responsible for specific
functionality. This makes it easy to maintain and update for us, whilst also
making it easy for you to use.

- [Exposed Model](@ref ExposedModelLibrary) responsible for both GUI
  specification as well as communicating variables between the GUI and your
  application. For cloud applications it makes use of a sub-component that
  translates the variables into an XML document following a special schema which
  is transmitted across the internet.

- The [Renderlist](@ref RenderListLibrary) component handles creation of WebGL renderlists and generating proxy geometry
  for use on the client side.

- The Controller component specifies the interfaces needed to create a
  Tinia-based application. It has several interfaces depending on the use-case of
  the application, and can easily be extended if need be. Typical interfaces are
  QTObserver for a desktop job and TrellObserver for cloud jobs.

- The Trell component is responsible for the client – server interactions
  through an Apache2 web server, and communicates with the TrellObserver using
  IPC.




