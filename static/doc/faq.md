Frequently asked questions{#sec_faq}
==========================

What is Tinia?
--------------
Tinia is a framework for developing applications in which the main computational
logic is done on a server, while the results is displayed on a client.

Through Tinia the application programmer can specify a GUI which Tinia may
convert to either a QT GUI or a JavaScript GUI (using the Dojo toolkit).

Tinia is well suited for OpenGL and GPGPU programming.

Tinia aims to provide high interactivity even in situations with high latency
and low bandwidth.



What is Tinia not?
------------------
- Tinia is not a subtitute for Remote Desktop, OnLive, Gaikai or similar services.
- Tinia is not QT, GTK or any other toolkit framework.
- Tinia is not ready for production use.

What is a Job in Tinia?
-----------------------
A [Job](@ref tinia::jobcontroller::Job) is the core of a Tinia program.
A Job by itself does not constitute a full program, but by using the Tinia program
it becomes a full program.

Is the ExposedModel thread safe?
--------------------------------
Yes. You can alter the ExposedModel from whichever thread you'd like. We make
sure GUI updating is done in the correct thread.

