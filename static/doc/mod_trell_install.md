Installing mod_trell {#sec_mod_trell_install}
======
If you've used the .deb package to install Tinia, you don't need to take
any extra steps to install mod_trell. But if you're installing Tinia through
other means, you'll have to manually install the mod_trell module in apache.

Some familiarty with how apache httpd
works is assumed, you should probably consult your distribution's documentation
on how to configure the apache web server.

First you need to load the trell module in apache, so you need to insert
the the line

`LoadModule trell_module <the path to libmod_trell.so>`

in the httpd.conf file.

You also need to set up the extra configuration options needed for mod_trell.
You should consult the "config/mod_trell.conf" file to see which options are
needed. A short description is provided here.

You typically want to edit the options `TrellMasterExe`, `TrellAppRoot`,
`TrellSchemaRoot` and `TrellJobWWWRoot`.

`TrellMasterExe` should be the full path to the executable trell_master,
`TrellAppRoot` should be the directory where you want to place your executables
you want mod_trell to run.

`TrellSchemaRoot` should point to the folder `tinia/schemas`, and
`TrellJobWWWRoo`should point to the folder `tinia/js`.

Once you've set up apache httpd correctly, you should restart the httpd service
and check out the [mod_trell web interface guide](@ref sec_mod_trell_gui).

