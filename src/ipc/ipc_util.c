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

#include <tinia/ipc/ipc_util.h>
#include <stdlib.h>
#include <ctype.h>

int
tinia_ipc_util_valid_jobid( tinia_ipc_msg_log_func_t  log_f,
                            void*                     log_d,
                            const char*               job_id )
{
    static const char* who = "tinia.ipc.util.valid_jobid";
    if( job_id == NULL ) {
        log_f( log_d, 0, who, "got nullptr." );
        return 0;
    }
    if( job_id[0] == '\0' ) {
        log_f( log_d, 0, who, "got empty string." );
        return 0;
    }
    int len;
    for(len=0; len<TINIA_IPC_JOBID_MAXLENGTH+1; len++ ) {
        int c = job_id[len];
        if( c == '\0' ) {
            break;
        }
        else if( !(isalnum(c)||(c=='_')) ) {
            log_f( log_d, 0, who, "Illegal character '%c' in job id.", c );
            return 0;
        }
    }
    if( len >= TINIA_IPC_JOBID_MAXLENGTH ) {
        log_f( log_d, 0, who, "Job id too long." );
        return 0;
    }
    return 1;
}
