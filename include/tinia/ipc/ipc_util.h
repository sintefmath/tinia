#pragma once
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


#ifdef __cplusplus
extern "C" {
#endif

#define TINIA_IPC_JOBID_MAXLENGTH 64

/** User-supplied callback that handles logging.
 *
 * \param[in] data     Optional data passed from callback supplier.
 * \param[in] level    Loglevel, 0 implies error, 1 implies warning, and 2
 *                     implies an informal message.
 * \param[in] who      Identifier of function that wants to log.
 * \param[in] message  Printf-formatted log message.
 * \param[in] ...      Arguments to log message.
 *
 */
typedef void (*tinia_ipc_msg_log_func_t)( void*        data,
                                          int          level,
                                          const char*  who,
                                          const char*  message, ... )
__attribute__((format(printf,4,5)))
;


/** Checks if job id is valid.
 *
 * Checks that:
 * - pointer is nonzero
 * - string is nonempty
 * - string is maximally TINIA_IPC_JOBID_MAXLENGTH long.
 * - string contains only alphanumeric characters or underscores.
 *
 * \param[in] log_f  Callback that handleds log messages.
 * \param[in] log_d  Optional data passed to log callback.
 * \param[in] job_id  Character string to check.
 *
 * \returns 1 if job_id is valid, zero otherwise.
 */
int
tinia_ipc_util_valid_jobid( tinia_ipc_msg_log_func_t  log_f,
                            void*                     log_d,
                            const char*               job_id );



#ifdef __cplusplus
}
#endif
