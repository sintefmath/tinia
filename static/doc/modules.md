Tina Components {#sec_modules}
============

The Tinia Framework is split into several components, each responsible for specific
functionality. This makes it easy to maintain and update for us, whilst also
making it easy for you to use.

- \subpage exposedmodellibrary "Exposed Model"
    responsible for both GUI
  specification as well as communicating variables between the GUI and your
  application. For cloud applications it makes use of a sub-component that
  translates the variables into an XML document following a special schema which
  is transmitted across the internet.

- The \subpage RenderListLibrary "Renderlist" component handles creation of WebGL renderlists and generating proxy geometry
  for use on the client side.

- The \subpage JobControllerLibrary "Controller component" specifies the interfaces needed to create a
  Tinia-based application. It has several interfaces depending on the use-case of
  the application, and can easily be extended if need be. Typical interfaces are
  QTObserver for a desktop job and TrellObserver for cloud jobs.

- The Trell component is responsible for the client  server interactions
  through an Apache2 web server, and communicates with the TrellObserver using
  IPC.
