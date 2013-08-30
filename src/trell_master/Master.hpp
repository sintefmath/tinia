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

#include <vector>
#include <string>
#include <unordered_map>
#include "tinia/trell/IPCController.hpp"
#include "Applications.hpp"

namespace tinia {
namespace trell {
namespace impl {
    

/** The master job
  *
  * The job management is offloaded from mod_trell to a specific master job. The
  * master job creates new jobs, kills jobs, and monitors the jobs.
  *
  */
class Master : public IPCController
{
public:

    /** Constructor
      *
      * \param for_real  Actually do forking etc (which is skipped in some test
      *                  scenarios).
      */
    Master( bool for_real = false );

protected:
    bool                                    m_for_real;
    std::string                             m_application_root;
    Applications                            m_applications;

    static const std::string                getApplicationRoot();
    
    /** The internal represenation of a job. */
    struct Job {
        /** The unique id of the job. */
        std::string                         m_id;
        /** Process id of job. */
        pid_t                               m_pid;
        /** The executable of the job. */
        std::string                         m_executable;
        /** Last state update from job. */
        TrellJobState                       m_state;
        /** Timestamp for last state update. */
        time_t                              m_last_ping;
        /** Application argument list (excluding application name). */
        std::vector<std::string>            m_args;
    };
    /** The set of managed jobs. */
    std::unordered_map<std::string, Job>    m_jobs;

    /** Helper struct to extract contents from XML messages sent by the client. */
    struct ParsedXML
    {
        enum {
            ACTION_NONE,
            ACTION_PING,
            ACTION_GET_SERVER_LOAD,
            ACTION_WIPE_JOB,
            ACTION_KILL_JOB,
            ACTION_ADD_JOB,
            ACTION_GET_JOB_LIST,
            ACTION_LIST_RENDERING_DEVICES,
            ACTION_LIST_APPLICATIONS
        }                           m_action;
        std::string                 m_job;
        std::string                 m_application;
        int                         m_timestamp;
        std::vector<std::string>    m_args;
        bool                        m_force;
        std::string                 m_session;
    };

    /** \copydoc MessageBox::init
      *
      * Invokes snarfMasterState.
      *
      */
    bool
    init();

    /** \copydoc MessageBox::periodic
      *
      * Runs through the list of jobs, and checks if the pids of non-terminated
      * jobs are valid. If the pid is not valid, the job is labeled as
      * terminated.
      */
    bool
    periodic();

    void
    often();
    
    /** \copydoc MessageBox::cleanup
      *
      * Invokes dumpMasterState.
      */
    void
    cleanup();


    /** Dump list of managed jobs to disc.
      *
      * The list of jobs are maintained in the xml file /tmp/trell_master_state,
      * such that the list of managed jobs can be recovered if the master job
      * is restarted.
      */
    void
    dumpMasterState();

    /** Read list of managed jobs from disc.
      *
      * \sa dumpMasterState.
      */
    void
    snarfMasterState();

    /** Encode the list of managed jobs as XML, either for transmissing to client or for disc storage. */
    const std::string
    encodeMasterState();

    /** Parse XML sent from the client.
      *
      * Helper function to decode the XML messages sent by the client.
      * \param data  The decoded data.
      * \param buf   The buffer that contains the XML document.
      * \param len   The size of the buffer that contains the XML document.
      */
    void
    parseXML( ParsedXML& data, char* buf, size_t len );

    /** Handles TRELL_MESSAGE_XML messages and might produce xml replies.
      *
      * \param out  The buffer to store the reply xml.
      * \param out_size  The size of output buffer, updated with the actual size
      *                  of the reply xml.
      * \param in        The buffer containing the input xml message.
      * \param in_size   The size of the input xml message.
      * \returns         Either TRELL_MESSAGE_XML, TRELL_MESSAGE_OK, or
      *                  TRELL_MESSAGE_ERROR.
      */
    TrellMessageType
    handleXML( char* out, size_t& out_size, const char* in, size_t in_size );

    /** Create a new job.
      *
      * \arg id  The server-wide unique id of the job.
      * \arg exe The path of the executable that implements the job.
      * \arg args Application arguments (excluding application binary).
      * \arg xml The XML message that requested the job to be created. Arguments
      *          can be passed here.
      */
    bool
    addJob( const std::string& id,
            const std::string& exe,
            const std::vector<std::string>& args,
            const std::string& xml );

    /** Kill a job.
      *
      * \arg id     The server-wide unique id of the job.
      * \arg force  If force is false, a suicidie request is sent to the job. If
      *             force is true, a SIGKILL is sent to the process.
      */
    bool
    killJob( const std::string& id, bool force );
    
    /** Make sure that IPC-stuff for the job is cleaned up. */
    void
    cleanJobRemains( const std::string& id );

    /** Remove a job from the list of managed jobs. */
    bool
    wipeJob( const std::string& id );


    /** Get an XML-coded string with the system load. */
    const std::string
    getLoad();



    /** Convenience function to get the current time. */
    const time_t
    getTime() const;


    /** Handle a raw trell message. */
    size_t
    handle( trell_message* msg, size_t buf_size );

    /** Updates the state of a job.
      *
      * \param job        The job id of the job to update.
      * \param state      The new state of the job.
      * \param heartbeat  If true, invocation originated from a heartbeat
      *                   request, and the last ping of the job is updated.
      * \returns True if job exists, false otherwise.
      */
    bool
    setJobState( const std::string& job, TrellJobState state, bool heartbeat=false );

    const std::string
    stderrPath( const std::string job ) const;

    const std::string
    stdoutPath( const std::string job ) const;

};

}
} // of namespace trell
} // of namespace tinia
