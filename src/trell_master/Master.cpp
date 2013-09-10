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

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <sstream>
#include <vector>
#include <fstream>
#include <libxml/xmlreader.h>
#include "Master.hpp"
#include <sys/mman.h>
#include <cstdarg>
#include <cstdio>
//#include <sys/stat.h>        /* For mode constants */
//#include <fcntl.h>           /* For O_* constants */

namespace {
static const std::string package = "Master";
    struct parse {
	enum NodeType {
        NODE_UNKNOWN,
        // Action nodes
        NODE_PING,
        NODE_GET_SERVER_LOAD,
        NODE_GET_JOB_LIST,
        NODE_KILL_JOB,
        NODE_WIPE_JOB,
        NODE_ADD_JOB,
        NODE_GRANT_ACCESS,
        NODE_REVOKE_ACCESS,
        NODE_LIST_RENDERING_DEVICES,
        NODE_LIST_APPLICATIONS,
        // Parameter nodes
        NODE_JOB,
        NODE_APPLICATION,
        NODE_RENDERING_DEVICE_ID,
        NODE_ARG,
        NODE_TIMESTAMP,
        NODE_FORCE,
        NODE_SESSION
	};
    };
    struct snarf {
	enum NodeType {
	    NODE_UNKNOWN,
	    NODE_JOBINFO,
	    NODE_JOB,
	    NODE_PID,
	    NODE_APPLICATION,
	    NODE_ARG,
	    NODE_STATE
	};
    };
}
namespace tinia {
namespace trell {
namespace impl {
    using std::vector;
    using std::string;
namespace {
static const std::string ret_header  = "<?xml version=\"1.0\"?>"
                                       "<reply "
                                       " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
                                       " xmlns=\"http://cloudviz.sintef.no/trell/1.0\""
                                       ">";
static const std::string ret_footer  = "</reply>";
static const std::string ret_pong    = ret_header + "<pong/>" + ret_footer;
static const std::string ret_success = ret_header + "<result>SUCCESS</result>" + ret_footer;
static const std::string ret_failure = ret_header + "<result>FAILURE</result>" + ret_footer;
}

namespace {


static sig_atomic_t sigchild_flag = 0;

static
void
handle_SIGCHLD( int )
{
    sigchild_flag = 1;
    std::cerr << "handle_SIGCHLD\n";
}
    
}

const std::string
Master::getApplicationRoot()
{
    const char* app_root = getenv( "TINIA_APP_ROOT" );
    if( app_root == NULL ) {
        std::cerr << "TINIA_APP_ROOT env variable not set!" << std::endl;
        exit( EXIT_FAILURE );
    }
    return app_root;
}


Master::Master( bool for_real )
: IPCController( true ),
  m_for_real( for_real ),
  m_applications( getApplicationRoot() ),
  m_rendering_devices( m_logger_callback, m_logger_data )
{
    m_application_root = std::string( getApplicationRoot() );
    if( m_logger_callback != NULL ) {
        m_logger_callback( m_logger_data, 2, package.c_str(),
                           "TINIA_APP_ROOT=%s", m_application_root.c_str() );
    }
}


size_t
Master::handle( trell_message* msg, size_t buf_size )
{
    TrellMessageType return_type = TRELL_MESSAGE_ERROR;
    size_t osize = 0;

    if( msg->m_type == TRELL_MESSAGE_XML ) {
        ParsedXML data;
        data.m_action = ParsedXML::ACTION_NONE;
        parseXML( data, msg->m_xml_payload, msg->m_size );

        string retval;
        switch( data.m_action ) {
        case ParsedXML::ACTION_NONE:
            break;
        case ParsedXML::ACTION_PING:
            retval = ret_pong;
            break;
        case ParsedXML::ACTION_GET_SERVER_LOAD:
            retval = getLoad();
            break;
        case ParsedXML::ACTION_WIPE_JOB:
            if( wipeJob( data.m_job ) ) {
                retval = ret_success;
            }
            else {
                retval = ret_failure;
            }
            break;
        case ParsedXML::ACTION_KILL_JOB:
            if( killJob( data.m_job, data.m_force ) ) {
                retval = ret_success;
            }
            else {
                retval = ret_failure;
            }
            break;
        case ParsedXML::ACTION_ADD_JOB:
            if( addJob( data.m_job,
                        data.m_application,
                        data.m_args,
                        data.m_rendering_devices,
                        string( msg->m_xml_payload, msg->m_size ) ) ) {
                retval = ret_success;
            }
            else {
                retval = ret_failure;
            }
            break;
        case ParsedXML::ACTION_GET_JOB_LIST:
            retval = encodeMasterState();
            break;
        case ParsedXML::ACTION_LIST_RENDERING_DEVICES:
            retval = ret_header
                   + m_rendering_devices.xml()
                   + ret_footer;
            break;
        case ParsedXML::ACTION_LIST_APPLICATIONS:
            m_applications.refresh();
            if( m_applications.timestamp() <= data.m_timestamp ) {
                retval = ret_success;
            }
            else {
                retval = ret_header
                       + m_applications.xml()
                       + ret_footer;
            }
            break;
        }
        
        if( !retval.empty() ) {

            size_t l = retval.size() + 1;
            if( TRELL_MSGHDR_SIZE + l < buf_size ) {
                return_type = TRELL_MESSAGE_XML;
                osize = l;
                strcpy( msg->m_xml_payload, retval.c_str() );
            }
            else {
                std::cerr << "Shmem buffer too small.\n";
            }
        }

    }
    else if( msg->m_type == TRELL_MESSAGE_HEARTBEAT ) {
        std::string job = msg->m_ping_payload.m_job_id;
        setJobState( job, msg->m_ping_payload.m_state, true );
        return_type = TRELL_MESSAGE_OK;
        osize = 0u;
    }


    msg->m_type = return_type;
    msg->m_size = osize;
    return osize + TRELL_MSGHDR_SIZE;
}

bool
Master::init( )
{
    if( IPCController::init() ) {
        snarfMasterState();

        // --- Install signal handlers ---------------------------------------------
        struct sigaction act;
        memset( &act, 0, sizeof(act) );
        sigemptyset( &act.sa_mask );
    
        act.sa_handler = handle_SIGCHLD;
        if( sigaction( SIGCHLD, &act, NULL ) != 0 ) {
            std::cerr << "sigaction/SIGCHLD failed: " << strerror( errno ) << "\n";
            return false;
        }
    
        // --- Unblock signals we want ---------------------------------------------
        sigset_t mask;
        sigemptyset( &mask );
        sigaddset( &mask, SIGCHLD );
        if( sigprocmask( SIG_UNBLOCK, &mask, NULL ) != 0 ) {
            std::cerr << "sigprocmask failed: " << strerror( errno ) << "\n";
            return false;
        }
        
        return true;
    }
    else {
        return false;
    }
}

bool
Master::periodic()
{
    std::cerr << "Periodic func...\n";
    bool ret = IPCController::periodic();
    dumpMasterState();
    for( auto it=m_jobs.begin(); it!=m_jobs.end(); ++it ) {
        if( !( (it->second.m_state == TRELL_JOBSTATE_TERMINATED_SUCCESSFULLY) ||
               (it->second.m_state == TRELL_JOBSTATE_TERMINATED_UNSUCCESSFULLY ) ) ) {
            if( kill( it->second.m_pid, 0 ) < 0 ) {
                std::cerr << "Job '" << it->second.m_id << "' cannot receive signals, probably dead.\n";
                setJobState( it->second.m_id, TRELL_JOBSTATE_TERMINATED_UNSUCCESSFULLY );
            }
            else {
                std::cerr << "Job '" << it->second.m_id << "' is probably alive.\n";
            }
        }
    }
    return ret;
}

void
Master::often()
{
    while( sigchild_flag != 0 ) {
        sigchild_flag = 0;
        
        std::cerr << "Got SIGCHLD, checking for dead children.\n";
        pid_t pid;
        while( (pid = waitpid( -1, NULL, WNOHANG )) > 0 ) {
            std::cerr << "pid=" << pid << " is dead.\n";
            bool found_job = false;
            for( auto it=m_jobs.begin(); (it!=m_jobs.end()) && (!found_job); ++it ) {
                if( it->second.m_pid == pid ) {
                    found_job = true;
                    if( it->second.m_state != TRELL_JOBSTATE_TERMINATED_SUCCESSFULLY ) {
                        setJobState( it->second.m_id, TRELL_JOBSTATE_TERMINATED_UNSUCCESSFULLY );
                        cleanJobRemains( it->second.m_id );
                    }
                }
            }
            if( !found_job ) {
                std::cerr << "Got dead child pid=" << pid << " not in records...?\n";
            }
        }
        
    }
    
}


void
Master::cleanup()
{
    dumpMasterState();
    IPCController::cleanup();
}

bool
Master::setJobState( const std::string& job, TrellJobState state, bool heartbeat )
{
    auto it = m_jobs.find( job );
    if( it != m_jobs.end() ) {
        TrellJobState old_state = it->second.m_state;
        it->second.m_state = state;
        if( heartbeat ) {
            it->second.m_last_ping = getTime();
        }
        if( old_state != state ) {
            dumpMasterState();
        }
        return true;
    }
    else {
        return false;
    }
}

void
Master::parseXML( ParsedXML& data, char* buf, size_t len )
{
    std::cerr << std::string(  buf, buf + len ) << "\n";
    
    xmlTextReaderPtr reader = xmlReaderForMemory( buf,
                                                  len,
                                                  "hetcomp.sintef.no/cloudviz",
                                                  NULL,
                                                  XML_PARSE_NOBLANKS );
    if( reader != NULL ) {
        vector<parse::NodeType> stack;
        for( int r=xmlTextReaderRead(reader); r==1; r=xmlTextReaderRead(reader)) {
            int type = xmlTextReaderNodeType( reader );
            // Begin element
            if( type == 1 ) {          // --- start of element
                xmlChar* name = xmlTextReaderLocalName( reader );
                if( name == NULL ) {
                    name = xmlStrdup( BAD_CAST "--" );
                }
		parse::NodeType n = parse::NODE_UNKNOWN;
                if( xmlStrEqual( name, BAD_CAST "ping" ) ) {
                    n = parse::NODE_PING;
                    data.m_action = ParsedXML::ACTION_PING;
                }
                else if( xmlStrEqual( name, BAD_CAST "getServerLoad" ) ) {
                    n = parse::NODE_GET_SERVER_LOAD;
                    data.m_action = ParsedXML::ACTION_GET_SERVER_LOAD;
                }
                else if( xmlStrEqual( name, BAD_CAST "getJobList" ) ) {
                    n = parse::NODE_GET_JOB_LIST;
                    data.m_action = ParsedXML::ACTION_GET_JOB_LIST;
                }
                else if( xmlStrEqual( name, BAD_CAST "killJob" ) ) {
                    n = parse::NODE_KILL_JOB;
                    data.m_action = ParsedXML::ACTION_KILL_JOB;
                }
                else if( xmlStrEqual( name, BAD_CAST "addJob" ) ) {
                    n = parse::NODE_ADD_JOB;
                    data.m_action = ParsedXML::ACTION_ADD_JOB;
                }
                else if( xmlStrEqual( name, BAD_CAST "wipeJob" ) ) {
                    n = parse::NODE_WIPE_JOB;
                    data.m_action = ParsedXML::ACTION_WIPE_JOB;
                }
                else if( xmlStrEqual( name, BAD_CAST "listRenderingDevices" ) ) {
                    n = parse::NODE_LIST_RENDERING_DEVICES;
                    data.m_action = ParsedXML::ACTION_LIST_RENDERING_DEVICES;
                }
                else if( xmlStrEqual( name, BAD_CAST "listApplications" ) ) {
                    n = parse::NODE_LIST_APPLICATIONS;
                    data.m_action = ParsedXML::ACTION_LIST_APPLICATIONS;
                }
                else if( xmlStrEqual( name, BAD_CAST "timestamp" ) ) {
                    n = parse::NODE_TIMESTAMP;
                }
                else if( xmlStrEqual( name, BAD_CAST "grantAccess" ) ) {
                    n = parse::NODE_GRANT_ACCESS;
                    data.m_action = ParsedXML::ACTION_NONE;
                }
                else if( xmlStrEqual( name, BAD_CAST "revokeAccess" ) ) {
                    n = parse::NODE_GRANT_ACCESS;
                    data.m_action = ParsedXML::ACTION_NONE;
                }
                else if( xmlStrEqual( name, BAD_CAST "job" ) ) {
                    n = parse::NODE_JOB;
                }
                else if( xmlStrEqual( name, BAD_CAST "application")) {
                    n = parse::NODE_APPLICATION;
                }
                else if( xmlStrEqual( name, BAD_CAST "arg" ) ) {
                    n = parse::NODE_ARG;
                }
                else if( xmlStrEqual( name, BAD_CAST "renderingDeviceId" ) ) {
                    n = parse::NODE_RENDERING_DEVICE_ID;
                }
                else if( xmlStrEqual( name, BAD_CAST "force")) {
                    n = parse::NODE_FORCE;
                }
                else if( xmlStrEqual( name, BAD_CAST "session")) {
                    n = parse::NODE_SESSION;
                }
                if( !xmlTextReaderIsEmptyElement( reader ) ) {
                    stack.push_back( n );
                }
            }
            // Element content
            else if( type == 3 ) {
                xmlChar* text = xmlTextReaderValue( reader );
                if( text != NULL ) {
                    switch( stack.back() ) {
                    case parse::NODE_JOB:
                        data.m_job = reinterpret_cast<const char*>( text );
                        break;
                    case parse::NODE_APPLICATION:
                        data.m_application = reinterpret_cast<const char*>( text );
                        break;
                    case parse::NODE_TIMESTAMP:
                        data.m_timestamp = atoi( reinterpret_cast<const char*>( text ) );
                        break;
                    case parse::NODE_ARG:
                        data.m_args.push_back( reinterpret_cast<const char*>( text ) );
                        break;
                    case parse::NODE_RENDERING_DEVICE_ID:
                        data.m_rendering_devices.push_back( reinterpret_cast<const char*>( text ) );
                        break;
                    case parse::NODE_FORCE:
                        if( xmlStrEqual( text, BAD_CAST "1" ) || xmlStrEqual( text, BAD_CAST "true" ) ) {
                            data.m_force = true;
                        }
                        else {
                            data.m_force = false;
                        }
                        break;
                    case parse::NODE_SESSION:
                        data.m_session = reinterpret_cast<const char*>( text );
                        break;
                    default:
                        break;
                    }
                    xmlFree( text );
                }
            }
            // Element end
            else if( type == 15 ) {
                stack.pop_back();
            }
        }
        xmlFreeTextReader( reader );
    }
}

const std::string
Master::encodeMasterState()
{
    std::stringstream o;
    o << ret_header;
    o << "<jobList>\n";
    for(auto it=m_jobs.begin(); it!=m_jobs.end(); ++it ) {
        const Job& job = it->second;
        o << "  <jobInfo>\n";
        o << "    <job>" << job.m_id << "</job>\n";
        o << "    <pid>" << job.m_pid << "</pid>\n";
        o << "    <application>" << job.m_executable << "</application>\n";
        for( auto kt=job.m_args.begin(); kt!=job.m_args.end(); ++kt ) {
            o << "    <arg>" << (*kt) << "</arg>\n";
        }
        o << "    <state updated=\"" << job.m_last_ping << "\">";
        switch( job.m_state ) {
        case TRELL_JOBSTATE_NOT_STARTED: o << "NOT_STARTED"; break;
        case TRELL_JOBSTATE_RUNNING: o << "RUNNING"; break;
        case TRELL_JOBSTATE_FINISHED: o << "FINISHED"; break;
        case TRELL_JOBSTATE_FAILED: o << "FAILED"; break;
        case TRELL_JOBSTATE_TERMINATED_SUCCESSFULLY: o << "TERMINATED_SUCCESSFULLY"; break;
        case TRELL_JOBSTATE_TERMINATED_UNSUCCESSFULLY: o << "TERMINATED_UNSUCCESSFULLY"; break;
        }
        o << "</state>\n";
        o << "    <allowed>";
        o << "</allowed>\n";
        o << "  </jobInfo>";
    }
    o << "</jobList>\n";
    o << ret_footer;
    return o.str();
}


void
Master::snarfMasterState()
{
    xmlTextReaderPtr reader = xmlReaderForFile( "/tmp/trell_master_state", NULL, XML_PARSE_NOBLANKS );
    if( reader != NULL ) {

        std::vector<snarf::NodeType> stack;

        Job job;
        unsigned int fields = 0u;

        for( int r=xmlTextReaderRead(reader); r==1; r=xmlTextReaderRead(reader)) {

            int type = xmlTextReaderNodeType( reader );
            if( type == 1 ) {
		snarf::NodeType type = snarf::NODE_UNKNOWN;
                xmlChar* name = xmlTextReaderLocalName( reader );
                if( name != NULL ) {
                    if( xmlStrEqual( name, BAD_CAST "jobInfo" ) ) {
                        type = snarf::NODE_JOBINFO;
                        job = Job();
                        fields = 0u;
                    }
                    else if( xmlStrEqual( name, BAD_CAST "job" ) ) {
                        type = snarf::NODE_JOB;
                    }
                    else if( xmlStrEqual( name, BAD_CAST "pid" ) ) {
                        type = snarf::NODE_PID;
                    }
                    else if( xmlStrEqual( name, BAD_CAST "application" ) ) {
                        type = snarf::NODE_APPLICATION;
                    }
                    else if( xmlStrEqual( name, BAD_CAST "arg" ) ) {
                        type = snarf::NODE_ARG;
                    }
                    else if( xmlStrEqual( name, BAD_CAST "state" ) ) {
                        type = snarf::NODE_STATE;
                    }
                    else if( xmlStrEqual( name, BAD_CAST "allowed" ) ) {
                        // TODO
                    }
                    xmlFree( name );
                }
                stack.push_back( type );
            }
            else if( type == 3 ) {
                xmlChar* text = xmlTextReaderValue( reader );
                if( text != NULL ) {
                    switch( stack.back() ) {
                    break;
                    case snarf::NODE_JOB:
                        job.m_id = reinterpret_cast<const char*>( text );
                        fields |= (1<<stack.back());
                        break;
                    case snarf::NODE_PID:
                        job.m_pid = atoi( reinterpret_cast<const char*>( text ) );
                        fields |= (1<<stack.back());
                        break;
                    case snarf::NODE_APPLICATION:
                        job.m_executable = reinterpret_cast<const char*>( text );
                        fields |= (1<<stack.back());
                        break;
                    case snarf::NODE_ARG:
                        job.m_args.push_back( reinterpret_cast<const char*>( text ) );
                        break;
                    case snarf::NODE_STATE:
                        if( xmlStrEqual( text, BAD_CAST "NOT_STARTED" ) ) {
                            job.m_state = TRELL_JOBSTATE_NOT_STARTED;
                            fields |= (1<<stack.back());
                        }
                        else if( xmlStrEqual( text, BAD_CAST "RUNNING" ) ) {
                            job.m_state = TRELL_JOBSTATE_RUNNING;
                            fields |= (1<<stack.back());
                        }
                        else if( xmlStrEqual( text, BAD_CAST "FINISHED" ) ) {
                            job.m_state = TRELL_JOBSTATE_FINISHED;
                            fields |= (1<<stack.back());
                        }
                        else if( xmlStrEqual( text, BAD_CAST "FAILED" ) ) {
                            job.m_state = TRELL_JOBSTATE_FAILED;
                            fields |= (1<<stack.back());
                        }
                        else if( xmlStrEqual( text, BAD_CAST "TERMINATED_SUCCESSFULLY" ) ) {
                            job.m_state = TRELL_JOBSTATE_TERMINATED_SUCCESSFULLY;
                            fields |= (1<<stack.back());
                        }
                        else if( xmlStrEqual( text, BAD_CAST "TERMINATED_UNSUCCESSFULLY" ) ) {
                            job.m_state = TRELL_JOBSTATE_TERMINATED_UNSUCCESSFULLY;
                            fields |= (1<<stack.back());
                        }
                        else {
                            std::cerr << "Unrecognized state: " << reinterpret_cast<const char*>( text );
                        }
                        break;
                    default:
                        break;
                    }
                    xmlFree( text );
                }
            }
            else if( type == 15 ) {
                if( stack.back() == snarf::NODE_JOBINFO ) {
                    unsigned int all = (1<<snarf::NODE_JOB) |
			               (1<<snarf::NODE_PID) |
			               (1<<snarf::NODE_APPLICATION) |
			               (1<<snarf::NODE_STATE);
                    if( fields == all ) {
                        auto it = m_jobs.find( job.m_id );
                        if( it != m_jobs.end() ) {
                            std::cerr << "Job " << job.m_id << " already recorded.\n";
                        }
                        else {
                            m_jobs[ job.m_id ] = job;
                        }
                    }
                    else {
                        std::cerr << "Missing fields: " << fields << "\n";
                    }
                }
                stack.pop_back();
            }
        }
        xmlFreeTextReader( reader );
    }
    std::cerr << "Snarfed previous state." << getTime() << "\n";
}

bool
Master::wipeJob( const std::string& id )
{
    auto it = m_jobs.find( id );
    if( it == m_jobs.end() ) {
        std::cerr << "wipeJob: id '" << id << "' not recognized.\n";
        return false;
    }
    if( !( (it->second.m_state == TRELL_JOBSTATE_TERMINATED_SUCCESSFULLY) ||
           (it->second.m_state == TRELL_JOBSTATE_TERMINATED_UNSUCCESSFULLY) ) ) {
        std::cerr << "wipeJob: id '" << id << "' has not terminated.\n";
        return false;
    }
    waitpid( it->second.m_pid, NULL, WNOHANG );


    m_jobs.erase( it );
    dumpMasterState();
    std::cerr << "wipeJob: wiped '" << id << "'.\n";
    return true;
}

bool
Master::killJob( const std::string& id, bool force )
{
    std::cerr << "invoked kill(" << id << ", " << force << ");\n";

    if( id.empty() ) {
        return false;
    }
    if( force ) {
        auto it = m_jobs.find( id );
        if( it != m_jobs.end() ) {
            if( kill( it->second.m_pid, SIGKILL ) == 0 ) {
                return true;
            }
            else {
                std::cerr << "kill(" << it->second.m_pid << ") failed:" << strerror(errno) << "\n";
                return false;
            }
        }
        else {
            killJob( id, false );
        }
    }
    else {
        if( sendSmallMessage( id, TRELL_MESSAGE_DIE ) == TRELL_MESSAGE_OK ) {
            return true;
        }
        else {
            return false;
        }
    }
    return true;
}

void
Master::cleanJobRemains( const std::string& id )
{
    std::string shmem_name      = "/" + id + "_shmem";
    std::string sem_lock_name   = "/" + id + "_lock";
    std::string sem_query_name  = "/" + id + "_query";
    std::string sem_reply_name  = "/" + id + "_reply";
    std::string sem_notify_name = "/" + id + "_notify";

    shm_unlink( shmem_name.c_str() );
    sem_unlink( sem_lock_name.c_str() );
    sem_unlink( sem_query_name.c_str() );
    sem_unlink( sem_reply_name.c_str() );
    sem_unlink( sem_notify_name.c_str() );
}


const std::string
Master::stderrPath( const std::string job ) const
{
    return std::string( "/tmp/" ) + job + ".stderr";
}

const std::string
Master::stdoutPath( const std::string job ) const
{
    return std::string( "/tmp/" ) + job + ".stdout";
}

bool
Master::addJob( const std::string& id,
                const std::string& exe,
                const std::vector<std::string>& args,
                const std::vector<std::string>& rendering_devices,
                const std::string& xml )
{
    if( id.empty() || exe.empty() ) {
        return false;
    }
    std::cerr << "ADDJOB " << xml << "\n";
    for(auto it=args.begin(); it!=args.end(); ++it ) {
        std::cerr << "ARG " << *it << "\n";
    }
    
    for(auto it=id.begin(); it!=id.end(); ++it ) {
        if( !(isalnum( *it ) || *it=='_' ) ) {
            std::cerr << "Illegal job id.\n";
            return false;
        }
    }
    for(auto it=exe.begin(); it!=exe.end(); ++it ) {
        if( !(isalnum( *it ) || *it=='_' ) ) {
            std::cerr << "Illegal executable path.\n";
            return false;
        }
    }
    auto it = m_jobs.find( id );
    if( it != m_jobs.end() ) {
        std::cerr << "Job already exist.\n";
        return false;
    }
    m_jobs[id] = Job();
    it = m_jobs.find( id );
    it->second.m_id = id;
    it->second.m_pid = 42;
    it->second.m_executable =  m_application_root + "/" + exe;
    it->second.m_state = TRELL_JOBSTATE_NOT_STARTED;
    it->second.m_last_ping = getTime();
    it->second.m_args = args;
    it->second.m_rendering_devices = rendering_devices;
    if( m_for_real ) {
        fsync( 1 );
        fsync( 2 );
        pid_t pid = fork();
        if( pid == -1 ) {
            // Failed to fork.
            it->second.m_pid = -1;
            it->second.m_state = TRELL_JOBSTATE_TERMINATED_UNSUCCESSFULLY;
        }
        else if( pid == 0 ) {
            int o = creat( stdoutPath( id ).c_str(), 0666 );
            dup2( o, 1 );
            close( o );

            int e = creat( stderrPath( id ).c_str(), 0666 );
            dup2( e, 2 );
            close( e );

            // create arguments
            std::vector<char*> arg;
            arg.push_back( strdup( it->second.m_executable.c_str() ) );
            for(auto kt=args.begin(); kt!=args.end(); ++kt) {
                arg.push_back( strdup( kt->c_str() ) );
            }
            arg.push_back( NULL );

            // copy and add to environment             
            std::string env_job_id    = "TINIA_JOB_ID=" + it->second.m_id;
            std::string env_master_id = "TINIA_MASTER_ID=" + getMasterID();
            std::vector<char*> env;
            env.push_back( strdup( env_job_id.c_str() ) );
            env.push_back( strdup( env_master_id.c_str() ) );

            if( !it->second.m_rendering_devices.empty() ) {
                std::string devices;
                for( std::vector<std::string>::iterator kt = it->second.m_rendering_devices.begin();
                     kt != it->second.m_rendering_devices.end(); ++kt )
                {
                    if( !devices.empty() ) {
                        devices.append( ";" );
                    }
                    devices.append( *kt );
                }
                devices = "TINIA_RENDERING_DEVICES=" + devices;
                env.push_back( strdup( devices.c_str() ) );
            }
            
            for( int i=0; environ[i] != NULL; i++ ) {
                if( strncmp( environ[i], "TINIA_", 6 ) != 0 ) {
                    env.push_back( strdup( environ[i] ) );
                }
            }
            env.push_back( NULL );
            
            // and run...
            execvpe( it->second.m_executable.c_str(),
                     reinterpret_cast<char* const*>( arg.data() ),
                     reinterpret_cast<char* const*>( env.data() ) );
            std::cerr << "Failed to exec: " << strerror(errno) << "\n";

            // Notify master that things went wrong

            messenger m;
            messenger_status_t mrv;

            mrv = messenger_init( &m, getMasterID().c_str() );
            if( mrv != MESSENGER_OK ) {
                std::cerr << __func__ << ": messenger_init(" << getMasterID() << ") failed: " << messenger_strerror( mrv ) << "\n";
            }
            else {
                mrv = messenger_lock( &m );

                if( mrv != MESSENGER_OK ) {
                    std::cerr << __func__ << ": messenger_lock(" << getMasterID() << ") failed: " << messenger_strerror( mrv ) << "\n";
                }
                else {
                    size_t msg_size = (it->second.m_id.length()+1) + offsetof( trell_message, m_ping_payload.m_job_id );
                    if( msg_size < m.m_shmem_size ) {
                        trell_message* msg = reinterpret_cast<trell_message*>( m.m_shmem_ptr );
                        msg->m_type = TRELL_MESSAGE_HEARTBEAT;
                        msg->m_size = msg_size - TRELL_MSGHDR_SIZE;
                        msg->m_ping_payload.m_state = TRELL_JOBSTATE_TERMINATED_UNSUCCESSFULLY;
                        strcpy( msg->m_ping_payload.m_job_id, it->second.m_id.c_str() );

                        mrv = messenger_post( &m, msg_size );
                        if( mrv != MESSENGER_OK ) {
                            std::cerr << __func__ << ": messenger_post("<<getMasterID()<<"): " << messenger_strerror( mrv ) << "\n";
                        }
                        else {
                            std::cerr << "Notified master.\n";
                        }
                    }
                    mrv = messenger_unlock( &m );
                    if( mrv != MESSENGER_OK ) {
                        std::cerr << __func__ << ": messenger_unlock("<<getMasterID()<<"): " << messenger_strerror( mrv ) << "\n";
                    }
                }
                messenger_status_t mrv = messenger_free( &m );
                if( mrv != MESSENGER_OK ) {
                    std::cerr << __func__ << ": messenger_free("<<getMasterID()<<"): " << messenger_strerror( mrv ) << "\n";
                }
            }
            std::cerr << "Exiting.\n";
            exit( EXIT_FAILURE );
        }
        else {
            it->second.m_pid = pid;
        }
    }
    dumpMasterState();
    return true;
}

void
Master::dumpMasterState()
{
    std::ofstream o;
    o.open( "/tmp/trell_master_state", std::ios_base::trunc );
    o << encodeMasterState();
    o.close();
}



const time_t
Master::getTime() const
{
    struct timeval rolex;
    gettimeofday( &rolex, NULL );
    return rolex.tv_sec;
}

const std::string
Master::getLoad()
{
    std::ifstream f( "/proc/loadavg" );
    std::stringstream o;
    const char* intervals[3] = { "1m", "5m", "15m" };
    o << ret_header;
    o << "<serverLoad>";
    for(size_t i=0; i<3; i++ ) {
        float l;
        f >> l;
        o << "<avg interval=\"" << intervals[i] << "\">" << l << "</avg>";
    }
    o << "</serverLoad>";
    o << ret_footer;
    return o.str();
}




} // of namespace impl
} // of namespace trell
} // of namespace tinia
