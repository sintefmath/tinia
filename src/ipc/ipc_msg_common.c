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
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>  // isalnum
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#ifdef __GNUC__
#include <execinfo.h>
#endif
#include "ipc_msg_internal.h"


int     ipc_msg_fake_shmem = 0;
pthread_mutex_t  ipc_msg_fake_shmem_lock = PTHREAD_MUTEX_INITIALIZER;
void*   ipc_msg_fake_shmem_ptr = NULL;
size_t  ipc_msg_fake_shmem_size = 0;
int     ipc_msg_fake_shmem_users = 0;


char*
ipc_msg_strerror_wrap( int errnum,
                       char* buf,
                       size_t buflen )
{
    return strerror(errnum); //strerror_r( errnum, buf, buflen );
}

void
tinia_ipc_msg_dump_backtrace( tinia_ipc_msg_log_func_t log_f, void* log_d )
{
    static const char* who = "tinia.ipc.msg.dump_backtrace";
#ifdef __GNUC__
    void* array[40];
    char** strings;
    
    size_t size, i;
    size = backtrace( array, 40 );
    strings = backtrace_symbols( array, size );
    for(i=0; i<size; i++) {
        log_f( log_d, 2, who, "%2ld: %s.", i, strings[i] );
    }
    free(strings);  
#else
    log_f( log_d, 0, who, "Not implemented" );    
#endif
}


int
ipc_msg_shmem_path( tinia_ipc_msg_log_func_t log_f,
                    void *log_d,
                    char* path,
                    const size_t n,
                    const char* jobid )
{
    static const char* who = "tinia.ipc.msg.shmem.path";

    if( path == 0 ) {
        log_f( log_d, 0, who, "Passed null-pointer in path.\n" );
        return -1;
    }
    else if( jobid == NULL) {
        log_f( log_d, 0, who, "Passed null-pointer in jobid.\n" );
        return -1;
    }

    static const char* prefix = "/tinia_shmem_";

    size_t i=0;
    for(; i<n && prefix[i] != '\0'; i++ ) {
        path[i] = prefix[i];
    }
    size_t k=i;
    for(; i<n && jobid[i-k] != '\0'; i++ ) {
        path[i] = jobid[i-k];
    }
    if( i < n ) {
        path[i] = '\0';
        return 0;
    }
    else {
        log_f( log_d, 0, who, "Insufficient space in buffer: jobid too long.\n" );
        return -1;
    }
}
