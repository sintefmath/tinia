mod_trell web interface {#sec_mod_trell_gui}
============================================

The mod_trell web interface may seem a little uninviting at first, and it's
probably not the most user friendly interface, but for simple testing purposes
it works.

A note about error checking
---
If at any time during this simple guide you experience problems with mod_trell,
you should check the text files `/tmp/trell_master.stdout` and
`/tmp/trell_master.stderr` for (hopefully) useful error messages. One can
also check `/var/log/apache/error_log` (or `/var/log/httpd/error_log`) for
more error messages marked with `mod_trell`.

Starting mod_trell master
---
After you've success fully set up mod_trell, pointing your browser to
http://localhost/trell/static (replace localhost with whatever hostname/IP
your mod_trell server is running on) should show something like this:
\image html mod_trell_start.png "Initial status of the mod_trell web interface"

The first thing you need to do is to press the "(Re)start" button. This should
not change anything, but hitting the
"Refresh" button to the right of the
"(Re)start" button after you've pressed "(Re)start" should show something like
this:
\image html mod_trell_refresh.png "mod_trell is now running"

A short note about terminology
---
In Tinia, a program made through the Tinia framework is frequently referenced
as a job. The idea being that a Job is a part of a complete program, the job
is not a whole program by itself, but by using the Tinia framework
components it becomes a full program.

Starting a new program / job
---
If you press the "Refresh" button below and to the left of "Running jobs"
you should see something like this
\image html mod_trell_refresh_job.png "mod_trell with refreshed job list"

Now mod_trell displays all running tinia server programs. As we haven't started
any programs yet, this list is empty.

To start a new program, we must write in a unique id in the "Job id" field. The
id can be any (reasonable short) text without spaces. The executable field
should be the name of the executable to be run. mod_trell will look for
executables in the directory specified by the apache-httpd option
`TrellAppRoot`.
If you've installed tinia through the .deb package, this will be
`/var/trell/apps`. For now, we'll use the simple test program `stest_job`.

Once you press "Commit" you should see something like this:
\image html mod_trell_starting_job.png "mod_trell is starting the new job."

If you wait a little while, or press "Refresh" below the "Interact" button,
you should see that the state field is changed to running:
\image html mod_trell_started_job.png "The job has been started."

You can interact with the program by pressing the "Interact" button, opening
up a new window showing something like this:
\image html mod_trell_interact_job.png "Interacting with the job."

### Errors when starting a job
If you didn't manage to start the job, you should check the
text files
`/tmp/<your_job_id>.stderr` and `/tmp/<your_job_id>.stdout` for error messages.

The most common error message is something similar to "Failed to open display
':0.0'". This typically stems from strict X restrictions. You can amend this
by running the command `xhost +` in a terminal window in your X session, or
by changing the user your Apache httpd server is run through.

Stopping jobs
---
To stop a job you should press the "Suicide" button in the "Running jobs" list.
After a short period of time, the state of the Job should change, showing
something like this
\image html mod_trell_killed_job.png "A job has exited."
By pressing "Wipe" the job is removed from the "Running jobs", thus freeing up
the id to be used again.
