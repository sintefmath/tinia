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
#ifndef TINIA_IPC_MESSENGER_INTERNAL_H
#define TINIA_IPC_MESSENGER_INTERNAL_H

#include <tinia/ipc/messenger.h>



#ifdef __cplusplus
extern "C" {
#endif


#define LOG_WRAP( s, level, where, ... ) \
do { \
    if( s->m_log_f != NULL ) { \
        s->m_log_f( s->m_log_d, level, where, __VA_ARGS__ ); \
    } \
} while( 0 )
#define LOG_ERROR( s, where, ... ) LOG_WRAP( s, 0, where, __VA_ARGS__ )
#define LOG_WARN( s, where, ... )  LOG_WRAP( s, 1, where, __VA_ARGS__ )
#define LOG_INFO( s, where, ... )  LOG_WRAP( s, 2, where, __VA_ARGS__ )


struct tinia_ipc_msg_header_struct {
    size_t  m_size;
    int     m_more;
} __attribute__((aligned(16)));

typedef struct tinia_ipc_msg_header_struct tinia_ipc_msg_header_t;


/** Server side of the ipc message passing. */
struct tinia_ipc_msg_server_struct
{
    volatile int            m_notify;       ///< Used to notify a notification.
    volatile int            m_end;          ///< Used to notify that the mainloop should break.
    void*                   m_shmem_ptr;    ///< Base pointer to the shared memory.
    size_t                  m_shmem_size;   ///< Size of all shared memory.
    tinia_ipc_msg_header_t* m_header_ptr;   ///< Pointer to header segment in shared memory.
    void*                   m_body_ptr;     ///< Pointer to the body segment (where data goes) in shared memory.
    size_t                  m_body_size;    ///< Size of body segment.
    sem_t*                  m_sem_lock;     ///< Mutex lock of the server.
    sem_t*                  m_sem_query;    ///< Incoming message part condition variable.
    sem_t*                  m_sem_reply;    ///< Outgoing message part condition variable.
    sem_t*                  m_sem_notify;   ///< Wait-on-notification condition variable.
    char                    m_name[256];    ///< Base name for server, fixed size to avoid malloc/free.
    tinia_ipc_msg_logger_t  m_log_f;     
    void*                   m_log_d;    
};

/** Client side of the ipc message passing. */
struct tinia_ipc_msg_client_struct
{
    int                     m_has_lock;
    void*                   m_shmem_ptr;    ///< Base pointer to the shared memory.
    size_t                  m_shmem_size;   ///< Size of all shared memory.
    volatile tinia_ipc_msg_header_t* m_header_ptr;   ///< Pointer to header segment in shared memory.
    void*                   m_body_ptr;     ///< Pointer to the body segment (where data goes) in shared memory.
    size_t                  m_body_size;    ///< Size of body segment.
    sem_t*                  m_lock;
    sem_t*                  m_query;
    sem_t*                  m_reply;
    sem_t*                  m_notify;
    tinia_ipc_msg_logger_t  m_log_f;
    void*                   m_log_d;    
};

#ifdef __cplusplus
}
#endif
#endif // TINIA_IPC_MESSENGER_INTERNAL_H
