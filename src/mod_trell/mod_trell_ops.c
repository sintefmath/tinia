/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <httpd.h>
#include <http_log.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "tinia/trell/trell.h"
#include "mod_trell.h"

#include "apr_strings.h"
#include "apr_env.h"

int
trell_ops_rpc_handle( trell_sconf_t* sconf, request_rec* r )
{
    apr_hash_t* args = trell_parse_args_uniq_key( r, r->args );
    if( args == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r,
                       "mod_trell: mod/rpc.xml without any arguments." );
        return HTTP_BAD_REQUEST;
    }

    const char* action = apr_hash_get( args, "action", APR_HASH_KEY_STRING );

    if( action == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r,
                       "mod_trell: mod/rpc.xml without action argument." );
        return HTTP_BAD_REQUEST;
    }
    else if( strcmp( action, "restart_master" ) == 0 ) {
        return trell_ops_do_restart_master( sconf, r );
    }
    else {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r,
                       "mod_trell: mod/rpc.xml got illegal action '%s'.", action );
        return HTTP_BAD_REQUEST;
    }
}

static const char* master_job_pidfile_path = "/tmp/trell_master.pid";


/** Kill a process with a given pid.
 *
 * Used to from the apache module thread to kill the trell master job, and 
 * we try to wait and determine if the process is dead.
 */
static int
trell_kill_process( trell_sconf_t* svr_conf,  request_rec* r, pid_t pid )
{
    int i;
    apr_status_t rv;

    // Shaky..?
    apr_proc_t proc;
    
    proc.pid = pid;
    proc.in = NULL;
    proc.out = NULL;
    proc.err = NULL;
    
    
    // This is slightly shaky as well, since we're not necessarily the parent
    // process, and if the child isn't waited upon, we have a defunct/zombie.
    
    rv = apr_proc_kill( &proc, SIGTERM );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, rv, r, "mod_trell: %s@%d", __FILE__, __LINE__ );
        return rv;
    }
    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r, "mod_trell: Sent pid=%d SIGTERM", proc.pid );
    apr_sleep( 1000 ); // Slight wait to let process hopefully terminate.
    for(i=0; i<3; i++ ) {
        // Check if child is dead. However, it is not safe to assume that the
        // master is child of current process.
        int exitcode;
        apr_exit_why_e why=0;
        rv = apr_proc_wait( &proc, &exitcode, &why, APR_NOWAIT );
        if( rv == APR_CHILD_DONE ) {
            ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r, "mod_trell: SIGTERM why=%d", why );
            return APR_SUCCESS;
        }
        
        // Wait failed, check to see if the messenger still lives... If not,
        // we assume it's dead.
        struct messenger msgr;
        messenger_status_t status = messenger_init( &msgr, svr_conf->m_master_id );
        if( status != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r, "mod_trell: failed to get master messenger, assuming master is dead." );
            return APR_SUCCESS;
        }
        messenger_free( &msgr );
        
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r, "mod_trell: Child not done, sleeping it=%d", i );
        apr_sleep( 1000000 );
    }
    

    rv = apr_proc_kill( &proc, SIGKILL );
    apr_sleep( 1000 ); // Slight wait to let process hopefully terminate.
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, rv, r, "mod_trell: %s@%d", __FILE__, __LINE__ );
        return rv;
    }
    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r, "mod_trell: Sent pid=%d SIGKILL", proc.pid );
    for(i=0; i<3; i++ ) {
        int exitcode;
        apr_exit_why_e why=0;
        rv = apr_proc_wait( &proc, &exitcode, &why, APR_NOWAIT );
        if( rv == APR_CHILD_DONE ) {
            ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r, "mod_trell: SIGKILL why=%d", why );
            return APR_SUCCESS;
        }

        struct messenger msgr;
        messenger_status_t status = messenger_init( &msgr, svr_conf->m_master_id );
        if( status != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r, "mod_trell: failed to get master messenger, assuming master is dead." );
            return APR_SUCCESS;
        }
        messenger_free( &msgr );
 
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r, "mod_trell: Child not done, sleeping it=%d", i );
        apr_sleep( 1000000 );
    }
    ap_log_rerror( APLOG_MARK, APLOG_CRIT, OK, r, "mod_trell: Failed to kill master job." );
}

/** Record the master job pid into a file on disc. */
static int
trell_set_master_pid( trell_sconf_t* svr_conf,  request_rec* r, pid_t pid )
{
    apr_status_t rv;

    apr_file_t* pidfile = NULL;
    rv = apr_file_open( &pidfile, master_job_pidfile_path,
                        APR_FOPEN_CREATE | APR_WRITE | APR_TRUNCATE | APR_XTHREAD,
                        APR_OS_DEFAULT, r->pool );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_CRIT, rv, r, "mod_trell: Failed to open master job pid file for writing." );
        return rv;
    }
    
    apr_file_printf( pidfile, "%d\n", pid );
    apr_file_close( pidfile );
    
    return APR_SUCCESS;
}


/** Read the master job pid from file on disc. */
static int
trell_get_master_pid( trell_sconf_t* svr_conf,  request_rec* r, pid_t* pid )
{
    apr_status_t rv;

    apr_file_t* pidfile = NULL;
    rv = apr_file_open( &pidfile, master_job_pidfile_path,
                        APR_READ, APR_OS_DEFAULT, r->pool );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, rv, r, "mod_trell: Failed to open master job pid file for reading." );
        return rv;
    }

    char* line = (char*)apr_palloc( r->pool, sizeof(unsigned char)*256 );
    rv = apr_file_gets( line, 256, pidfile );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, rv, r, "mod_trell: %s@%d", __FILE__, __LINE__ );
        return rv;
    }
    *pid = apr_atoi64( line );
    return APR_SUCCESS;
}

/** Try to start a process running the master job. */
static int
trell_start_master( trell_sconf_t* svr_conf,  request_rec* r )
{
    apr_status_t rv;
    
    // Open file into which master job stdout is piped.
    apr_file_t* master_stdout = NULL;
    rv = apr_file_open( &master_stdout,
                        apr_psprintf( r->pool, "/tmp/%s.stdout", svr_conf->m_master_id ),
                        APR_FOPEN_CREATE | APR_WRITE | APR_TRUNCATE | APR_XTHREAD,
                        APR_OS_DEFAULT, r->pool );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_CRIT, rv, r, "mod_trell: Failed to open master job stdout" );
        return rv;
    }
        
    // Open file into which master job stderr is piped.
    apr_file_t* master_stderr = NULL;
    rv = apr_file_open( &master_stderr,
                        apr_psprintf( r->pool, "/tmp/%s.stderr", svr_conf->m_master_id ),
                        APR_FOPEN_CREATE | APR_WRITE | APR_TRUNCATE | APR_XTHREAD,
                        APR_OS_DEFAULT, r->pool );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_CRIT, rv, r, "mod_trell: Failed to open master job stderr" );
        return rv;
    }

    // Create process attribute and set file handles for pipes
    apr_procattr_t* procattr;
    
    rv = apr_procattr_create( &procattr, r->pool );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_CRIT, rv, r, "mod_trell: apr_procattr_create failed" );
        return rv;
    }
    
    rv = apr_procattr_child_out_set( procattr, master_stdout, NULL );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_CRIT, rv, r, "mod_trell: apr_procattr_child_out_set failed" );
        return rv;
    }
    
    rv = apr_procattr_child_err_set( procattr, master_stderr, NULL );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_CRIT, rv, r, "mod_trell: apr_procattr_child_err_set failed" );
        return rv;
    }
    
    rv = apr_procattr_cmdtype_set( procattr, APR_PROGRAM );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_CRIT, rv, r, "mod_trell: apr_procattr_cmdtype_set failed" );
        return rv;
    }


    const char* env[7] = {
        apr_psprintf( r->pool, "TINIA_JOB_ID=%s",    svr_conf->m_master_id ),
        apr_psprintf( r->pool, "TINIA_MASTER_ID=%s", svr_conf->m_master_id ),
        apr_psprintf( r->pool, "TINIA_APP_ROOT=%s",  svr_conf->m_app_root_dir ),
        NULL,   // PATH
        NULL,   // LD_LIBRARY_PATH
        NULL,   // DISPLAY
        NULL
    };

    // Copy PATH and LD_LIBRARY_PATH from the current environement, if set. It
    // might be an idea to pass these variables through the apache-config,
    // allowing more detailded control on jobs.
    int p = 3;
    char* PATH = NULL;
    if( (apr_env_get( &PATH, "PATH", r->pool ) == APR_SUCCESS ) &&
            (PATH != NULL) )
    {
        env[p++] = apr_psprintf( r->pool, "PATH=%s", PATH );
    } 
    char* LD_LIBRARY_PATH = NULL;
    if( (apr_env_get( &LD_LIBRARY_PATH, "LD_LIBRARY_PATH", r->pool ) == APR_SUCCESS ) &&
            (LD_LIBRARY_PATH != NULL) )
    {
        env[p++] = apr_psprintf( r->pool, "LD_LIBRARY_PATH=%s", LD_LIBRARY_PATH );
    }
    char* DISPLAY = NULL;
    if( (apr_env_get( &DISPLAY, "DISPLAY", r->pool ) == APR_SUCCESS ) &&
            (DISPLAY != NULL) )
    {
        env[p++] = apr_psprintf( r->pool, "DISPLAY=%s", DISPLAY );
    }
    
    // Set up arguments            
    const char* args[2] = {
        svr_conf->m_master_exe,
        NULL
    };

    
    apr_proc_t newproc;
    rv = apr_proc_create( &newproc,
                          svr_conf->m_master_exe,
                          args,
                          env,
                          procattr,
                          r->pool );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_CRIT, rv, r, "mod_trell: Failed to execute '%s'", svr_conf->m_master_exe );
        return rv;
    }
    
    rv = trell_set_master_pid( svr_conf, r, newproc.pid );
    if( rv != APR_SUCCESS ) {
        return rv;
    }
    
    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r, "mod_trell: Master running at pid %d", newproc.pid );
    return APR_SUCCESS;    
}

/** Run the master-job start/restart-procedure. */
int
trell_ops_do_restart_master( trell_sconf_t* svr_conf,  request_rec* r )
{
    int success = 0;


    
    
    
    sem_t* trell_lock = NULL;

    trell_lock = sem_open( "/trell_lock", O_CREAT, 0666, 1 );
    if( trell_lock != SEM_FAILED ) {
        struct timespec timeout;
        clock_gettime( CLOCK_REALTIME, &timeout );
        timeout.tv_nsec += 10;
        if( sem_timedwait( trell_lock, &timeout ) == 0 ) {

            // There is an issue here if the existing master wasn't created by
            // the current process (which is not unlikely). That is probably why
            // wait doesn't manage to unzombify the child.

            // First, check if there is a master running and kill it
            pid_t pid;
            if( trell_get_master_pid( svr_conf, r, &pid ) == APR_SUCCESS ) {
                ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r, "mod_trell: Found existing pid: %d", pid );
                if( pid > 0 ) {
                    trell_kill_process( svr_conf, r, pid );
                }
            }
            
            // Start master
            if( trell_start_master( svr_conf, r ) ) {
                success = 1;
            }
            else {
                success = 0;
            }
            
            // Release lock
            sem_post( trell_lock );
        }
        sem_close( trell_lock );
    }
    if( success ) {
        return trell_send_xml_success( svr_conf, r );
    }
    else {
        return trell_send_xml_failure( svr_conf, r );
    }
}
