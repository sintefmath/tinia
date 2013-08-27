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

Master::Master( bool for_real )
    : IPCController( true ),
      m_for_real( for_real )
{
    const char* app_root = getenv( "TINIA_APP_ROOT" );
    if( app_root == NULL ) {
        std::cerr << "TINIA_APP_ROOT env variable not set!" << std::endl;
        exit( EXIT_FAILURE );
    }
    m_application_root = std::string( app_root );

    std::cerr << "TINIA_APP_ROOT=" << m_application_root << std::endl;
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
    else if( msg->m_type == TRELL_MESSAGE_ARGS ) {
        std::string job = msg->m_args_payload.m_job_id;
        std::cerr << "Got request for args from '" << job << "'.\n";
        auto it = m_jobs.find( job );
        if( it == m_jobs.end() ) {
            std::cerr << "Cannot find job '" << job << "'.\n";
        }
        else {
            const string& xml = it->second.m_xml;
            if( xml.empty() ) {
                std::cerr << "XML for job '" << job << "' is empty.\n";
            }
            else {
                if( buf_size <= TRELL_MSGHDR_SIZE + xml.size() + 1 ) {
                    std::cerr << "Buffer too small.\n";
                }
                else {
                    return_type = TRELL_MESSAGE_XML;
                    osize = xml.size()+1;
                    strcpy( msg->m_xml_payload, xml.c_str() );
                }
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
Master::init( const std::string& xml )
{
    if( IPCController::init(xml) ) {
        snarfMasterState();
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
    xmlTextReaderPtr reader = xmlReaderForMemory( buf,
                                                  len,
                                                  "hetcomp.sintef.no/cloudviz",
                                                  NULL,
                                                  XML_PARSE_NOBLANKS );
    if( reader != NULL ) {
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
            // Parameter nodes
            NODE_JOB,
            NODE_APPLICATION,
            NODE_ARG,
            NODE_FORCE,
            NODE_SESSION
        };

        vector<NodeType> stack;
        for( int r=xmlTextReaderRead(reader); r==1; r=xmlTextReaderRead(reader)) {
            int type = xmlTextReaderNodeType( reader );
            // Begin element
            if( type == 1 ) {          // --- start of element
                xmlChar* name = xmlTextReaderLocalName( reader );
                if( name == NULL ) {
                    name = xmlStrdup( BAD_CAST "--" );
                }
                NodeType n = NODE_UNKNOWN;
                if( xmlStrEqual( name, BAD_CAST "ping" ) ) {
                    n = NODE_PING;
                    data.m_action = ParsedXML::ACTION_PING;
                }
                else if( xmlStrEqual( name, BAD_CAST "getServerLoad" ) ) {
                    n = NODE_GET_SERVER_LOAD;
                    data.m_action = ParsedXML::ACTION_GET_SERVER_LOAD;
                }
                else if( xmlStrEqual( name, BAD_CAST "getJobList" ) ) {
                    n = NODE_GET_JOB_LIST;
                    data.m_action = ParsedXML::ACTION_GET_JOB_LIST;
                }
                else if( xmlStrEqual( name, BAD_CAST "killJob" ) ) {
                    n = NODE_KILL_JOB;
                    data.m_action = ParsedXML::ACTION_KILL_JOB;
                }
                else if( xmlStrEqual( name, BAD_CAST "addJob" ) ) {
                    n = NODE_ADD_JOB;
                    data.m_action = ParsedXML::ACTION_ADD_JOB;
                }
                else if( xmlStrEqual( name, BAD_CAST "wipeJob" ) ) {
                    n = NODE_WIPE_JOB;
                    data.m_action = ParsedXML::ACTION_WIPE_JOB;
                }
                else if( xmlStrEqual( name, BAD_CAST "grantAccess" ) ) {
                    n = NODE_GRANT_ACCESS;
                    data.m_action = ParsedXML::ACTION_NONE;
                }
                else if( xmlStrEqual( name, BAD_CAST "revokeAccess" ) ) {
                    n = NODE_GRANT_ACCESS;
                    data.m_action = ParsedXML::ACTION_NONE;
                }
                else if( xmlStrEqual( name, BAD_CAST "job" ) ) {
                    n = NODE_JOB;
                }
                else if( xmlStrEqual( name, BAD_CAST "application")) {
                    n = NODE_APPLICATION;
                }
                else if( xmlStrEqual( name, BAD_CAST "arg" ) ) {
                    n = NODE_ARG;
                }
                else if( xmlStrEqual( name, BAD_CAST "force")) {
                    n = NODE_FORCE;
                }
                else if( xmlStrEqual( name, BAD_CAST "session")) {
                    n = NODE_SESSION;
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
                    case NODE_JOB:
                        data.m_job = reinterpret_cast<const char*>( text );
                        break;
                    case NODE_APPLICATION:
                        data.m_application = reinterpret_cast<const char*>( text );
                        break;
                    case NODE_ARG:
                        data.m_args.push_back( reinterpret_cast<const char*>( text ) );
                        break;
                    case NODE_FORCE:
                        if( xmlStrEqual( text, BAD_CAST "1" ) || xmlStrEqual( text, BAD_CAST "true" ) ) {
                            data.m_force = true;
                        }
                        else {
                            data.m_force = false;
                        }
                        break;
                    case NODE_SESSION:
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
        enum NodeType {
            NODE_UNKNOWN,
            NODE_JOBINFO,
            NODE_JOB,
            NODE_PID,
            NODE_APPLICATION,
            NODE_ARG,
            NODE_STATE
        };
        std::vector<NodeType> stack;

        Job job;
        unsigned int fields = 0u;

        for( int r=xmlTextReaderRead(reader); r==1; r=xmlTextReaderRead(reader)) {

            int type = xmlTextReaderNodeType( reader );
            if( type == 1 ) {
                NodeType type = NODE_UNKNOWN;
                xmlChar* name = xmlTextReaderLocalName( reader );
                if( name != NULL ) {
                    if( xmlStrEqual( name, BAD_CAST "jobInfo" ) ) {
                        type = NODE_JOBINFO;
                        job = Job();
                        fields = 0u;
                    }
                    else if( xmlStrEqual( name, BAD_CAST "job" ) ) {
                        type = NODE_JOB;
                    }
                    else if( xmlStrEqual( name, BAD_CAST "pid" ) ) {
                        type = NODE_PID;
                    }
                    else if( xmlStrEqual( name, BAD_CAST "application" ) ) {
                        type = NODE_APPLICATION;
                    }
                    else if( xmlStrEqual( name, BAD_CAST "arg" ) ) {
                        type = NODE_ARG;
                    }
                    else if( xmlStrEqual( name, BAD_CAST "state" ) ) {
                        type = NODE_STATE;
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
                    case NODE_JOB:
                        job.m_id = reinterpret_cast<const char*>( text );
                        fields |= (1<<stack.back());
                        break;
                    case NODE_PID:
                        job.m_pid = atoi( reinterpret_cast<const char*>( text ) );
                        fields |= (1<<stack.back());
                        break;
                    case NODE_APPLICATION:
                        job.m_executable = reinterpret_cast<const char*>( text );
                        fields |= (1<<stack.back());
                        break;
                    case NODE_ARG:
                        job.m_args.push_back( reinterpret_cast<const char*>( text ) );
                        break;
                    case NODE_STATE:
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
                if( stack.back() == NODE_JOBINFO ) {
                    unsigned int all = (1<<NODE_JOB) |
                                       (1<<NODE_PID) |
                                       (1<<NODE_APPLICATION) |
                                       (1<<NODE_STATE);
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
    it->second.m_xml = xml;
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

            std::string env_job_id    = "TINIA_JOB_ID=" + it->second.m_id;
            std::string env_master_id = "TINIA_MASTER_ID=" + getMasterID();
            
            const char* env[3] = {
                env_job_id.c_str(),
                env_master_id.c_str(),
                NULL
            };
            
            
            execle( it->second.m_executable.c_str(),
                    it->second.m_executable.c_str(),
                    it->second.m_id.c_str(),
                    getMasterID().c_str(),
                    NULL,
                    env );
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
