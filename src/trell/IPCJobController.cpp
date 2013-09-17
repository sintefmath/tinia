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

#include <iostream>
#include <cstring>
#include "tinia/trell/IPCJobController.hpp"
#include "tinia/model/ExposedModelLock.hpp"
#include "tinia/model/impl/xml/XMLHandler.hpp"

// This should not be included here, but will be fixed when interface is
// cleaned up (that is when IPCJobController is rinsed of all opengl-methods)
#include "tinia/jobcontroller/OpenGLJob.hpp"

namespace tinia {
namespace trell {
namespace {
    static const std::string package = "IPCJobController";
} // of anonymous namespace



IPCJobController::IPCJobController( bool is_master )
    : IPCController( is_master ),
      m_job( NULL ), m_updateOngoing(false)
{}

IPCJobController::~IPCJobController()
{
   if(m_model.get() != NULL)
   {
      m_model->removeStateListener(this);
      m_model->removeStateSchemaListener(this);
   }
}

void
IPCJobController::setJob(jobcontroller::Job *job)
{
    m_job = job;
    m_model = job->getExposedModel();
    m_model->addStateListener(this);
    m_model->addStateSchemaListener(this);
}

bool
IPCJobController::init()
{
    bool ipcControllerResponse = IPCController::init( );
    bool jobResponse = m_job->init( );
    m_xmlHandler = new model::impl::xml::XMLHandler(m_job->getExposedModel());

    return ipcControllerResponse && jobResponse;
}

bool
IPCJobController::periodic()
{
    return IPCController::periodic() && m_job->periodic();
}

void
IPCJobController::cleanup()
{
    m_job->cleanup();
    IPCController::cleanup();
}

bool
IPCJobController::onGetSnapshot( char*               buffer,
                               TrellPixelFormat    pixel_format,
                               const size_t        width,
                               const size_t        height,
                               const std::string&  session,
                               const std::string&  key )
{
    return false;
}

bool
IPCJobController::onGetRenderlist( size_t&             result_size,
                                 char*               result_buffer,
                                 const size_t        result_buffer_size,
                                 const std::string&  session,
                                 const std::string&  key,
                                 const std::string&  timestamp )
{
    return false;
}


bool
IPCJobController::onGetExposedModelUpdate( size_t&             result_size,
                                   char*               result_buffer,
                                   const size_t        result_buffer_size,
                                   const std::string&  session,
                                   const unsigned int  revision )
{

    result_size = m_xmlHandler->getExposedModelUpdate( result_buffer, result_buffer_size, revision );
    return result_size > 0;
}

bool
IPCJobController::onUpdateState( const char*         buffer,
                               const size_t        buffer_size,
                               const std::string&  session )
{

   bool retVal = false;
   { // Scope for modelLock;
      model::ExposedModelLock lock(m_model);

      // We don't want to be notified of updates when we're updating ourselves
      m_updateOngoing = true;
      retVal = m_xmlHandler->updateState( buffer, buffer_size );
   }
   // Now is probably a good time to update:
   // Note: This is thread safe, the worst that can happen is that we post two
   // updates to the client (instead of one) [in other words: no updates are lost]
   m_updateOngoing = false;
   notify();
   return retVal;
}

size_t
IPCJobController::handle( trell_message* msg, size_t msg_size, size_t buf_size )
{
    std::string session;
    std::string key;
    std::string timestamp;
    unsigned int revision;

    TrellPixelFormat format;
    int w, h;
    size_t retsize = 0u;

//    char buf[97];
//    for(int i=0; i<96; i++) {
//        int t = ((char*)msg)[i];
//        
//        if( isalnum(t) ) {
//            buf[i]=t;
//        }
//        else if (t==0) {
//            buf[i] = (i%10)+'0';
//        }
//        else {
//            buf[i] = '-';
//        }
//    }
//    buf[96] = '\0';
//    m_logger_callback( m_logger_data, 2, package.c_str(),
//                       "> %s", buf );
    
    switch( msg->m_type ) {

    case TRELL_MESSAGE_DIE:
    {
        m_logger_callback( m_logger_data, 2, package.c_str(),
                           "Received suggestion to commit suicide." );
        fail();
        volatile tinia_msg_t* r = (tinia_msg_t*)msg;
        r->type = TRELL_MESSAGE_OK;
        return sizeof(*r);
    }
        break;

    case TRELL_MESSAGE_GET_POLICY_UPDATE:
    {
        session = "undefined";
        revision = msg->m_get_model_update_payload.m_revision;

        volatile tinia_msg_t* reply = (tinia_msg_t*)msg;
        size_t result_size;
       
        if( onGetExposedModelUpdate( result_size,
                                     (char*)msg + sizeof(*reply),
                                     buf_size - sizeof(*reply),
                                     session,
                                     revision ) )
        {
            m_logger_callback( m_logger_data, 2, package.c_str(),
                               "Queried for policy, returning updates (has_revision=%d).", revision );
            reply->type = TRELL_MESSAGE_XML;
            return result_size + sizeof(*reply);
        }
        else {
            m_logger_callback( m_logger_data, 2, package.c_str(),
                               "Queried for policy, no updates available (has_revision=%d).", revision );
            msg->m_type = TRELL_MESSAGE_OK;
            return sizeof(*reply);
        }
    }
        break;

    case TRELL_MESSAGE_GET_SNAPSHOT:
        if( 1 ) {
            tinia_msg_get_snapshot_t* q = (tinia_msg_get_snapshot_t*)msg;

            format  = q->pixel_format;
            w       = q->width;
            h       = q->height;
            session = std::string( q->session_id );
            key     = std::string( q->key );
            
            if( format != TRELL_PIXEL_FORMAT_BGR8 ) {
                m_logger_callback( m_logger_data, 0, package.c_str(),
                                   "Queried for snapshot, unsupported image format %d.", (int)format );
            }
            else if( buf_size <= 3*w*h+sizeof(tinia_msg_image_t) ) {
                m_logger_callback( m_logger_data, 0, package.c_str(),
                                   "Queried for snapshot, buffer too small." );
                volatile tinia_msg_t* r = (tinia_msg_t*)msg;
                r->type = TRELL_MESSAGE_ERROR;
                return sizeof(*r);
            }
            else if( !onGetSnapshot( (char*)msg + sizeof(tinia_msg_image_t),
                                     format, w, h, session, key ) ) {
                m_logger_callback( m_logger_data, 0, package.c_str(),
                                   "Queried for snapshot, rendering error." );
            }
            else {
                m_logger_callback( m_logger_data, 2, package.c_str(),
                                   "Queried for snapshot, ok." );
                volatile tinia_msg_image_t* r = (tinia_msg_image_t*)msg;
                r->msg.type = TRELL_MESSAGE_IMAGE;
                r->width = w;
                r->height = h;
                r->pixel_format = format;
                return sizeof(*r) + 3*w*h; // size of msg + payload
            }
            volatile tinia_msg_t* r = (tinia_msg_t*)msg;
            r->type = TRELL_MESSAGE_ERROR;
            return sizeof(*r);
        }
        break;

    case TRELL_MESSAGE_GET_RENDERLIST:
    {
        session   = std::string( msg->m_get_renderlist.m_session_id );
        key       = std::string( msg->m_get_renderlist.m_key );
        timestamp = std::string( msg->m_get_renderlist.m_timestamp );
        
        volatile tinia_msg_t* reply = (tinia_msg_t*)msg;
        size_t result_size;
        if( onGetRenderlist( result_size,
                             (char*)msg + sizeof(*reply),
                             buf_size - sizeof(*reply),
                             session,
                             key,
                             timestamp ) )
        {
            reply->type = TRELL_MESSAGE_XML;
            m_logger_callback( m_logger_data, 2, package.c_str(),
                               "Queried for renderlist, ok." );
            return result_size + sizeof(*reply);
        }
        else {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Queried for renderlist, error occured." );
            msg->m_type = TRELL_MESSAGE_ERROR;
            return sizeof(*reply);
        }
    }
        break;

    case TRELL_MESSAGE_UPDATE_STATE:
    {
        volatile tinia_msg_t* reply = (tinia_msg_t*)msg;

        session = "undefined";
        if( onUpdateState( (char*)msg + sizeof(tinia_msg_t),
                           msg_size - sizeof(tinia_msg_t),
                           session ) )
        {
            reply->type = TRELL_MESSAGE_OK;
            m_logger_callback( m_logger_data, 2, package.c_str(),
                               "Update state, ok (msg_size=%d).", msg_size );
            return sizeof(*reply);
        }
        else {
            reply->type = TRELL_MESSAGE_ERROR;
            m_logger_callback( m_logger_data, 2, package.c_str(),
                               "Update state, failure." );
            return sizeof(*reply);
        }
    }
        break;

    case TRELL_MESSAGE_GET_SCRIPTS:
    {
        volatile tinia_msg_t* reply = (tinia_msg_t*)msg;
        size_t result_size;
        if ( onGetScripts( result_size,
                           (char*)msg + sizeof(*reply),
                           buf_size - sizeof(*reply) ) )
        {
            reply->type = TRELL_MESSAGE_SCRIPT;
            return result_size + sizeof(*reply);
        }
        else {
            reply->type = TRELL_MESSAGE_ERROR;
            return sizeof(*reply);
        }
    }
        break;

    default:
        m_logger_callback( m_logger_data, 0, package.c_str(),
                           "Received unknown message: type=%d, size=%d.",
                           msg->m_type, msg->m_size );
        msg->m_type = TRELL_MESSAGE_ERROR;
        msg->m_size = 0u;
        retsize = TRELL_MSGHDR_SIZE;
        break;
    }

    return retsize;
}



} // of namespace trell

void trell::IPCJobController::stateElementModified(model::StateElement *stateElement)
{
   // We only want to do something if we're not updating the model ourselves
   // (otherwise we'll post an update on completion)

   if(!m_updateOngoing)
   {
      notify();
   }
}

void trell::IPCJobController::stateSchemaElementAdded(model::StateSchemaElement *stateSchemaElement)
{

   if(!m_updateOngoing)
   {
      notify();
   }
}

void trell::IPCJobController::stateSchemaElementRemoved(model::StateSchemaElement *stateSchemaElement)
{

   if(!m_updateOngoing)
   {
      notify();
   }
}

void trell::IPCJobController::stateSchemaElementModified(model::StateSchemaElement *stateSchemaElement)
{

   if(!m_updateOngoing)
   {
      notify();
   }
}

} // of namespace tinia

