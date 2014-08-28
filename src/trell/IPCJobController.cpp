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
IPCJobController::handle( tinia_msg_t* msg, size_t msg_size, size_t buf_size )
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
    
    switch( msg->type ) {

    case TRELL_MESSAGE_DIE:
    {
        m_logger_callback( m_logger_data, 2, package.c_str(),
                           "Received suggestion to commit suicide." );
        fail();
        //volatile tinia_msg_t* r = (tinia_msg_t*)msg;
        msg->type = TRELL_MESSAGE_OK;
        return sizeof(tinia_msg_t);
    }
        break;

    case TRELL_MESSAGE_GET_POLICY_UPDATE:
    {
        tinia_msg_get_exposed_model_t* msg_get_exposed_model = (tinia_msg_get_exposed_model_t*)msg;
        
        session = "undefined";
        revision = msg_get_exposed_model->revision;

        size_t result_size;
        if( onGetExposedModelUpdate( result_size,
                                     (char*)msg + sizeof(tinia_msg_xml_t),
                                     buf_size - sizeof(tinia_msg_xml_t),
                                     session,
                                     revision ) )
        {
#ifdef DEBUG
            m_logger_callback( m_logger_data, 2, package.c_str(),
                               "Queried for policy, returning updates (has_revision=%d).", revision );
#endif
            tinia_msg_xml_t* reply = (tinia_msg_xml_t*)msg;
            reply->msg.type = TRELL_MESSAGE_XML;
            return result_size + sizeof(tinia_msg_xml_t);
        }
        else {
#ifdef DEBUG
            m_logger_callback( m_logger_data, 2, package.c_str(),
                               "Queried for policy, no updates available (has_revision=%d).", revision );
#endif
            msg->type = TRELL_MESSAGE_OK;
            return sizeof(tinia_msg_t);
        }
    }
        break;

    case TRELL_MESSAGE_GET_SNAPSHOT:
    {
        tinia_msg_get_snapshot_t* q = (tinia_msg_get_snapshot_t*)msg;

        format  = q->pixel_format;
        w       = q->width;
        h       = q->height;
        session = std::string( q->session_id );
        key     = std::string( q->key );

        m_logger_callback( m_logger_data, 2, package.c_str(), "jny IPCJobController::handle About to call onGetSnapshot, key = %s", key.c_str() );
        std::string key_list_string;
        m_job->getExposedModel()->getElementValue( "viewer_keys", key_list_string );
        m_logger_callback( m_logger_data, 2, package.c_str(), "jny IPCJobController::handle key_list_string = %s", key_list_string.c_str() );
        std::vector<std::string> key_list;
        boost::split( key_list, key_list_string, boost::is_any_of(" ") );

        size_t data_size = 0;
        switch ( format ) {
            case TRELL_PIXEL_FORMAT_BGR8:
                data_size = 3*w*h;
            break;
            case TRELL_PIXEL_FORMAT_BGR8_CUSTOM_DEPTH:
                // We would really like to have the size of each canvas associated with the keys we have, but this information is currently
                // not available. This will only work for equally sized canvases then, and no problem will be checked for or detected if it
                // is not the case!
                data_size = 4*((3*w*h+3)/4) * 2 + sizeof(float)*16*2; // Two padded images + 2 matrices
                // ... times the number of keys:
                data_size *= key_list.size();
            break;
            default:
                m_logger_callback( m_logger_data, 0, package.c_str(),
                                   "Queried for snapshot, unsupported image format %d.", (int)format );
                tinia_msg_t* reply = (tinia_msg_t*)msg;
                reply->type = TRELL_MESSAGE_ERROR;
            return sizeof(tinia_msg_t);
        }

        if ( buf_size <= data_size + sizeof(tinia_msg_image_t) ) {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Queried for snapshot, buffer too small." );
            tinia_msg_t* reply = (tinia_msg_t*)msg;
            reply->type = TRELL_MESSAGE_ERROR;
            return sizeof(tinia_msg_t);
        }

        // Looping through all keys and grabbing GL-content
        char *buf = (char*)msg + sizeof(tinia_msg_image_t);
        for (size_t i=0; i<1; i++) { // key_list.size(); i++) {
            key = key_list[i]; // Overriding the key gotten from the 'msg' parameter!
            m_logger_callback( m_logger_data, 2, package.c_str(), "jny IPCJobController::handle   key[%d] = %s", i, key.c_str() );

            if ( onGetSnapshot(buf, format, w, h, session, key) ) {
#ifdef DEBUG
                m_logger_callback( m_logger_data, 2, package.c_str(),
                                   "Queried for snapshot, ok. format = %d", format );
#endif
                if ( format == TRELL_PIXEL_FORMAT_BGR8_CUSTOM_DEPTH ) {
                    // In order to let trell_pass_reply_png_bundle() pacakge both images and the transformation matrices, we now write the
                    // latter two into the buffer.
                    tinia::model::Viewer viewer;
                    m_model->getElementValue( key, viewer );
                    if ( ( (unsigned long)buf ) % 4 != 0 ) {
                        m_logger_callback( m_logger_data, 2, package.c_str(), "Ouch. Non-aligned buffer %d", ( (unsigned long)buf ) % 4 );
                    }
                    buf += 4*((3*w*h+3)/4) * 2;
                    float *float_buf = (float *)buf;
                    for (size_t i=0; i<15; i++) {
                        float_buf[   i] = viewer.modelviewMatrix[i];
                    }
                    for (size_t i=0; i<15; i++) {
                        float_buf[16+i] = viewer.projectionMatrix[i];
                    }
                    buf += 2*16*sizeof(float); // Ready for the next canvas.
                }
            } else {
                m_logger_callback( m_logger_data, 0, package.c_str(), "Queried for snapshot, rendering error." );
                tinia_msg_t* reply = (tinia_msg_t*)msg;
                reply->type = TRELL_MESSAGE_ERROR;
                return sizeof(tinia_msg_t);
            }
        } // end of loop over keys

        // Actually more of an assertion:
        if ( buf > (char*)msg + sizeof(tinia_msg_image_t) + data_size ) {
            m_logger_callback( m_logger_data, 0, package.c_str(), "Buffer overflow, looks like a serious bug." );
            tinia_msg_t* reply = (tinia_msg_t*)msg;
            reply->type = TRELL_MESSAGE_ERROR;
            return sizeof(tinia_msg_t);
        }

        tinia_msg_image_t* reply = (tinia_msg_image_t*)msg;
        reply->msg.type     = TRELL_MESSAGE_IMAGE;
        reply->width        = w;
        reply->height       = h;
        reply->pixel_format = format;
        m_logger_callback( m_logger_data, 2, package.c_str(), "data_size = %d", data_size );
        return sizeof(tinia_msg_image_t) + data_size; // size of msg + payload
    }
    break;

    case TRELL_MESSAGE_GET_RENDERLIST:
    {
        tinia_msg_get_renderlist_t* msg_get_renderlist = (tinia_msg_get_renderlist_t*)msg;
        
        session   = std::string( msg_get_renderlist->session_id );
        key       = std::string( msg_get_renderlist->key );
        timestamp = std::string( msg_get_renderlist->timestamp );
        
        size_t result_size;
        if( onGetRenderlist( result_size,
                             (char*)msg + sizeof(tinia_msg_xml_t),
                             buf_size - sizeof(tinia_msg_xml_t),
                             session,
                             key,
                             timestamp ) )
        {
            tinia_msg_xml_t* reply = (tinia_msg_xml_t*)msg;
            reply->msg.type = TRELL_MESSAGE_XML;
#ifdef DEBUG
            m_logger_callback( m_logger_data, 2, package.c_str(),
                               "Queried for renderlist, ok." );
#endif
            return result_size + sizeof(tinia_msg_xml_t);
        }
        else {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Queried for renderlist, error occured." );
            msg->type = TRELL_MESSAGE_ERROR;
            return sizeof(tinia_msg_t);
        }
    }
        break;

    case TRELL_MESSAGE_UPDATE_STATE:
    {
        tinia_msg_update_exposed_model_t* query = (tinia_msg_update_exposed_model_t*)msg;
        session = query->session_id;

        if( onUpdateState( (char*)msg + sizeof(*query),
                           msg_size - sizeof(*query),
                           session ) )
        {
            //volatile tinia_msg_t* reply = (tinia_msg_t*)msg;
            msg->type = TRELL_MESSAGE_OK;
#ifdef DEBUG
            m_logger_callback( m_logger_data, 2, package.c_str(),
                               "Update state, ok (msg_size=%d).", msg_size );
#endif
            return sizeof(tinia_msg_t);
        }
        else {
            //volatile tinia_msg_t* reply = (tinia_msg_t*)msg;
            msg->type = TRELL_MESSAGE_ERROR;
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Update state, failure." );
            return sizeof(tinia_msg_t);
        }
    }
        break;

    case TRELL_MESSAGE_GET_SCRIPTS:
    {
        
        //volatile tinia_msg_t* reply = (tinia_msg_t*)msg;
        size_t result_size;
        if ( onGetScripts( result_size,
                           (char*)msg + sizeof(tinia_msg_script_t),
                           buf_size - sizeof(tinia_msg_script_t) ) )
        {
#ifdef DEBUG
            m_logger_callback( m_logger_data, 2, package.c_str(),
                               "Queried for scripts, ok (result_size=%ld).", result_size );
#endif
            tinia_msg_script_t* reply = (tinia_msg_script_t*)msg;
            reply->msg.type = TRELL_MESSAGE_SCRIPT;
            return sizeof(tinia_msg_script_t) + result_size;
        }
        else {
            m_logger_callback( m_logger_data, 0, package.c_str(),
                               "Queried for scripts, failed." );
            msg->type = TRELL_MESSAGE_ERROR;
            return sizeof(tinia_msg_t);
        }
    }
        break;

    default:
        m_logger_callback( m_logger_data, 0, package.c_str(),
                           "Received unknown message: type=%d, size=%d.",
                           msg->type, msg_size );
        msg->type = TRELL_MESSAGE_ERROR;
        retsize = sizeof(tinia_msg_t);
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

