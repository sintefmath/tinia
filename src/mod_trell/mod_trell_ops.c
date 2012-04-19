#include <httpd.h>
#include <http_log.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "trell/trell.h"
#include "mod_trell.h"


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
            FILE* pidfile = fopen( "/tmp/trell_master.pid", "r" );
            if( pidfile != NULL ) {
                ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r,
                               "mod_trell: Found existing pid-file" );
                int pid;
                int fscanf_res = fscanf( pidfile, "%d", &pid );
		if(fscanf_res == EOF)
		  fprintf(stderr, "whoops, error getting pid\n");

                fclose( pidfile );
                if( pid > 0 ) {
                    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r,
                                   "mod_trell: Existing pid = %d", pid );
                    if( kill( pid, SIGINT ) == 0 ) {
                        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r,
                                       "mod_trell: Sent SIGINT" );

                        // Give the process up to 3 secs to shut down
                        int i;
                        for( i=0; i<3; i++) {
                            int status;
                            if( waitpid( pid, &status, WNOHANG ) == pid ) {
                                break;
                            }
                            ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r,
                                           "mod_trell: Sleeping (%i)", i );
                            sleep( 1 );
                        }
                        if( kill( pid, SIGKILL ) == 0 ) {
                            ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r,
                                           "mod_trell: Sent SIGKILL" );
                            for( i=0; i<3; i++) {
                                int status;
                                if( waitpid( pid, &status, WNOHANG ) == pid ) {
                                    break;
                                }
                                ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r,
                                               "mod_trell: Sleeping (%i)", i );
                                sleep( 1 );
                            }
                        }
                    }
                }
            }

            // Start master
            const char* exe = svr_conf->m_master_exe;

            ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r,
                           "mod_trell: Preparing to fork and execute '%s'", exe );

            // Step 1: Flush stdout and stderr, to avoid duplicate log messages.
            fsync( 1 );
            fsync( 2 );
            // Step 2: Fork
            pid_t pid =  fork();
            if( pid == -1 ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, OK, r,
                               "mod_trell: Failed to fork" );
            }
            else if( pid == 0 ) {
                char buf[256];

                // Fork successful, we are the child. Redirect IO to files.
                snprintf( buf, sizeof(buf), "/tmp/%s.stdout", svr_conf->m_master_id );
                int o = creat( buf, 0666 );
                dup2( o, 1 );
                close( o );

                snprintf( buf, sizeof(buf), "/tmp/%s.stderr", svr_conf->m_master_id );
                int e = creat( buf, 0666 );
                dup2( e, 2 );
                close( e );
                execl( exe,                      // binary
                       exe,                      // binary is first arg as by convention
                       svr_conf->m_master_id,    // name of job to start
                       svr_conf->m_master_id,    // id of master job
                       svr_conf->m_app_root_dir, // app root directory
                       NULL );
                fprintf( stderr, "Failed to exec master.\n" );
                exit( EXIT_FAILURE );
            }
            else {
                // Fork successful, we are the parent. Record pid.
                FILE* pidfile = fopen( "/tmp/trell_master.pid", "w" );
                if( pidfile != NULL ) {
                    fprintf( pidfile, "%d\n", pid );
                    fclose( pidfile );
                }
                ap_log_rerror( APLOG_MARK, APLOG_NOTICE, OK, r, "mod_trell: Master running at pid %d", pid );
                success = 1;
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
