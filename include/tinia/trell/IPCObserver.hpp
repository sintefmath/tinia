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

#pragma once

#include <string>
#include <semaphore.h>
#include "messenger.h"
#include "trell.h"
#include "tinia/jobobserver/Observer.hpp"
#include "tinia/model/StateListener.hpp"
#include "tinia/model/StateSchemaListener.hpp"

class JobWrapper;
namespace tinia {
namespace trell {

/** Base class implementing message-passing destination point.
  *
  * mod_trell is based around passing messages between processes through shared
  * memory. This class implements functionality required to accept such
  * messages.
  *
  * Should not be used directly, but rather through Job or InteractiveJob.
  */
class IPCObserver : public jobobserver::Observer
{
    friend class ::JobWrapper;
public:

    /** Constuctor.
      *
      * \param is_master  Should be true for the master job and false for all
      *                   other jobs. The master shouldn't send ping-signales
      *                   etc. to itself.
      */
    IPCObserver( bool is_master = false );

    /** Destructor. */
    virtual
    ~IPCObserver();


    /** Entry point of the message box.
      *
      * The message box is set up by ivoking this function. The control of the
      * caller thread is taken, and this function does not return until the job
      * is finished.
      *
      * The correct places to add custom code is by overriding init, periodic,
      * and cleanup.
      */
    int
    run( int argc, char**argv );


    /** Force the application to fail and shut down (called by signal handler). */
    void
    failHard();

    /** Terminate job and label its completion as successful.
      *
      * This function is thread-safe (I hope). Should be called
      * by the subclass code when it wants to terminate.
      */
    void
    finish();

    /** Terminate job and label its completion as unsuccessful.
      *
      * This function is thread-safe (I hope). Should be called by the subclass
      * when something wrong as happend, and the job wants to just give up.
      */
    void
    fail();



    /** Notify long-pollers that they might fetch status. */
    void
    notify();

protected:



    /** Hook right before the process state changes from NOT_STARTED to RUNNING.
      *
      * Insert init code by overriding this. However, it is important to chain
      * with superclass, that is:
      * \code
      * bool
      * Foo::init( const std::string& xml ) {
      *     if( Super::init( xml ) ) {
      *         // ... do stuff ...
      *         return true or false;
      *     }
      *     else {
      *         return false;
      *     }
      * }
      * \endcode
      *
      * \param xml  A string that contains the xml document that was passed from
      *             the client starting this job. This might contain extra nodes
      *             that contains arguments.
      * \returns    True if everything is OK, or false if the process should
      *             terminate.
      */
    virtual
    bool
    init( const std::string& xml );

    /** Hook that is invoked every now and then.
      *
      * Not necessarily useful for most jobs. It is invoked at regular
      * intervals. Can be overriden, but it is important to chain with super-
      * class, that is:
      * \code
      * bool
      * Foo::periodic( ) {
      *     if( Super::periodic( ) ) {
      *         // ... do stuff ...
      *         return true or false;
      *     }
      *     else {
      *         return false;
      *     }
      * }
      * \endcode
      *
      * \returns True if everything is OK, or false if the process should
      *          terminate.
      */
    virtual
    bool
    periodic();


    /** Hook that is invoked just before the job terminates.
      *
      * A good place for cleanup-code, as the name implies. When overridden,
      * remember to chain with super-class, that is:
      * \code
      * void
      * Foo::cleanup( ) {
      *     // ... do stuff ...
      *     Super::cleanup( );
      * }
      * \endcode
      *
      */
    virtual
    void
    cleanup();






    /** Handle an incoming message.
      *
      * Invoked when a raw message arrives at the message ports. User should
      * not need to override this.
      *
      * \param msg       Pointer to the incoming message (and where to store the
      *                  outgoing reply).
      * \param buf_size  The size of the buffer, limiting the size of the
      *                  outgoing reply.
      * \returns The size of the reply message.
      */
    virtual
    size_t
    handle( trell_message* msg, size_t buf_size ) = 0;

    /** Convenience function to send a message without payload to a message box.
      *
      * \param message_box_id   The id of the message box.
      * \param query            The message type of the query.
      * \returns                The message type of the reply.
      */
    TrellMessageType
    sendSmallMessage( const std::string& message_box_id, TrellMessageType query );

    /** Get the id of the master job. */
    const std::string&
    getMasterID() { return m_master_id; }


private:
    pid_t           m_ipc_pid;

    /** The pid of the process that should invoke the cleanup functions (to
      * avoid clashes when fork succeds, but execl fails. */
    int             m_cleanup_pid;
    /** The id of this message box. */
    std::string     m_id;
    /** True of this job is the master job. */
    bool            m_is_master;
    /** The id of the master job. */
    std::string     m_master_id;
    /** The current state of this job/messagebox */
    TrellJobState   m_job_state;
    /** Pointer to the shared memory of this message box. */
    void*           m_shmem_ptr;
    /** The size (in bytes) of the shared memory of this message box. */
    size_t          m_shmem_size;
    /** The system-wide name of the shared memory of this message box. */
    std::string     m_shmem_name;
    /** The semaphore that can lock this message box. */
    sem_t*          m_sem_lock;
    /** The system-wide name of the semaphore that can lock this message box. */
    std::string     m_sem_lock_name;
    /** The semaphore that signals a pending incoming message. */
    sem_t*          m_sem_query;
    /** The system-wide name of the semaphore that signals a pending incoming message. */
    std::string     m_sem_query_name;

    /** The semaphore that signals a pending reply in response to the incoming message. */
    sem_t*          m_sem_reply;
    /** The system-wide name of the semaphore that singals a pending reply. */
    std::string     m_sem_reply_name;

    volatile bool   m_notify;
    sem_t*          m_sem_notify;
    std::string     m_sem_notify_name;

    /** A messenger to the master job's message box. */
    messenger       m_master_mbox;

    static bool
    createSharedMemory( void** memory, size_t* memory_size, const std::string& name, const size_t size );

    static void
    deleteSharedMemory( void** memory, size_t* memory_size, const std::string& name );

    static bool
    createSemaphore( sem_t** semaphore, const std::string& name );

    static void
    deleteSemaphore( sem_t** semaphore, const std::string& name );

    /** Sends a heartbeat message to the master job. */
    bool
    sendHeartBeat();

    /** Cleans up shared memory and semaphores. */
    void
    shutdown();

};


} // of namespace trell
} // of namespace tinia
