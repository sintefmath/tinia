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
#include "tinia/trell/IPCJobObserver.hpp"
#include "tinia/model/ExposedModelLock.hpp"

// This should not be included here, but will be fixed when interface is
// cleaned up (that is when IPCJobObserver is rinsed of all opengl-methods)
#include "tinia/jobobserver/OpenGLJob.hpp"

namespace tinia {
namespace Trell {


IPCJobObserver::IPCJobObserver( bool is_master )
    : IPCObserver( is_master ),
      m_job( NULL ), m_updateOngoing(false)
{}

IPCJobObserver::~IPCJobObserver()
{
   if(m_model.get() != NULL)
   {
      m_model->removeStateListener(this);
      m_model->removeStateSchemaListener(this);
   }
}

void
IPCJobObserver::setJob(jobobserver::Job *job)
{
    m_job = job;
    m_model = job->getExposedModel();
    m_model->addStateListener(this);
    m_model->addStateSchemaListener(this);
}

bool
IPCJobObserver::init( const std::string& xml )
{
    bool ipcObserverResponse = IPCObserver::init( xml );
    bool jobResponse = m_job->init( );
    m_xmlHandler = new modelxml::XMLHandler(m_job->getExposedModel());

    return ipcObserverResponse && jobResponse;
}

bool
IPCJobObserver::periodic()
{
    return IPCObserver::periodic() && m_job->periodic();
}

void
IPCJobObserver::cleanup()
{
    m_job->cleanup();
    IPCObserver::cleanup();
}

bool
IPCJobObserver::onGetSnapshot( char*               buffer,
                               TrellPixelFormat    pixel_format,
                               const size_t        width,
                               const size_t        height,
                               const std::string&  session,
                               const std::string&  key )
{
    return false;
}

bool
IPCJobObserver::onGetRenderlist( size_t&             result_size,
                                 char*               result_buffer,
                                 const size_t        result_buffer_size,
                                 const std::string&  session,
                                 const std::string&  key,
                                 const std::string&  timestamp )
{
    return false;
}


bool
IPCJobObserver::onGetExposedModelUpdate( size_t&             result_size,
                                   char*               result_buffer,
                                   const size_t        result_buffer_size,
                                   const std::string&  session,
                                   const unsigned int  revision )
{

    result_size = m_xmlHandler->getExposedModelUpdate( result_buffer, result_buffer_size, revision );
    return result_size > 0;
}

bool
IPCJobObserver::onUpdateState( const char*         buffer,
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
IPCJobObserver::handle( trell_message* msg, size_t buf_size )
{
    std::string session;
    std::string key;
    std::string timestamp;
    unsigned int revision;

    TrellPixelFormat format;
    int w, h;
    size_t retsize = 0u;

    switch( msg->m_type ) {

    case TRELL_MESSAGE_DIE:
        std::cerr << "Received suggestion to commit suicide.\n";
        fail();
        msg->m_type = TRELL_MESSAGE_OK;
        msg->m_size = 0u;
        retsize = TRELL_MSGHDR_SIZE;
        break;

    case TRELL_MESSAGE_GET_POLICY_UPDATE:
        session = "undefined";
        revision = msg->m_get_model_update_payload.m_revision;
        if( onGetExposedModelUpdate( msg->m_size,
                               msg->m_xml_payload,
                               buf_size - TRELL_MSGHDR_SIZE,
                               session,
                               revision ) )
        {
            msg->m_type = TRELL_MESSAGE_XML;
            retsize = TRELL_MSGHDR_SIZE + msg->m_size;
        }
        else {
            msg->m_type = TRELL_MESSAGE_OK;
            retsize = TRELL_MSGHDR_SIZE;
        }
        break;

    case TRELL_MESSAGE_GET_SNAPSHOT:
        format  = msg->m_get_snapshot.m_pixel_format;
        w       = msg->m_get_snapshot.m_width;
        h       = msg->m_get_snapshot.m_height;
        session = std::string( msg->m_get_snapshot.m_session_id );
        key     = std::string( msg->m_get_snapshot.m_key );
        if( format == TRELL_PIXEL_FORMAT_BGR8 ) {
            size_t image_size = 3*w*h;
            retsize = TRELL_MESSAGE_IMAGE_SIZE + image_size;
            if( retsize < buf_size ) {
                if( onGetSnapshot( msg->m_image.m_data,
                                   format, w, h, session, key ) )
                {
                    msg->m_type = TRELL_MESSAGE_IMAGE;
                }
                else {
                    msg->m_type = TRELL_MESSAGE_ERROR;
                    msg->m_size = 0;
                    retsize = TRELL_MSGHDR_SIZE;
                }
            }
            else {
                msg->m_type = TRELL_MESSAGE_ERROR;
                msg->m_size = 0;
                retsize = TRELL_MSGHDR_SIZE;
            }
        }
        else {
            msg->m_type = TRELL_MESSAGE_ERROR;
            msg->m_size = 0;
            retsize = TRELL_MSGHDR_SIZE;
        }
        break;

    case TRELL_MESSAGE_GET_RENDERLIST:
        session   = std::string( msg->m_get_renderlist.m_session_id );
        key       = std::string( msg->m_get_renderlist.m_key );
        timestamp = std::string( msg->m_get_renderlist.m_timestamp );
        if( onGetRenderlist( msg->m_size,
                             msg->m_xml.m_xml,
                             buf_size - TRELL_MESSAGE_XML_SIZE,
                             session,
                             key,
                             timestamp ) )
        {
            msg->m_type = TRELL_MESSAGE_XML;
            retsize = TRELL_MESSAGE_XML_SIZE + msg->m_size;
        }
        else {
            msg->m_type = TRELL_MESSAGE_ERROR;
            retsize = TRELL_MSGHDR_SIZE;
        }
        break;

    case TRELL_MESSAGE_UPDATE_STATE:
        session = "undefined";
        if( onUpdateState( msg->m_xml_payload, msg->m_size, session ) ) {
            msg->m_type = TRELL_MESSAGE_OK;
            retsize = TRELL_MSGHDR_SIZE;
        }
        else {
            msg->m_type = TRELL_MESSAGE_ERROR;
            retsize = TRELL_MSGHDR_SIZE;
        }
        break;

    default:
        msg->m_type = TRELL_MESSAGE_ERROR;
        msg->m_size = 0u;
        retsize = TRELL_MSGHDR_SIZE;
        break;
    }

    return retsize;
}



} // of namespace Trell

void Trell::IPCJobObserver::stateElementModified(model::StateElement *stateElement)
{
   // We only want to do something if we're not updating the model ourselves
   // (otherwise we'll post an update on completion)

   if(m_updateOngoing)
   {
      notify();
   }
}

void Trell::IPCJobObserver::stateSchemaElementAdded(model::StateSchemaElement *stateSchemaElement)
{

   if(m_updateOngoing)
   {
      notify();
   }
}

void Trell::IPCJobObserver::stateSchemaElementRemoved(model::StateSchemaElement *stateSchemaElement)
{

   if(m_updateOngoing)
   {
      notify();
   }
}

void Trell::IPCJobObserver::stateSchemaElementModified(model::StateSchemaElement *stateSchemaElement)
{

   if(m_updateOngoing)
   {
      notify();
   }
}

} // of namespace tinia

