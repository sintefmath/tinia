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

#include <httpd.h>
#include <http_log.h>
#include <http_protocol.h>
#include <util_filter.h>
#include <unistd.h>
#include <time.h>
#include <apr_strings.h>

#include "tinia/trell/trell.h"
#include "mod_trell.h"

int
trell_handle_get_script(trell_sconf_t           *sconf,
                            request_rec             *r,
                            trell_dispatch_info_t   *dispatch_info)
{


    int retval = HTTP_INTERNAL_SERVER_ERROR;

    messenger_t msgr;
    messenger_status_t mrv;

    mrv = messenger_init( &msgr, dispatch_info->m_jobid );
    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_init('%s') failed: %s",
                       dispatch_info->m_jobid, messenger_strerror( mrv ) );
        return HTTP_NOT_FOUND;
    }
    else {
        mrv = messenger_lock( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_lock('%s') failed: %s",
                           dispatch_info->m_jobid, messenger_strerror( mrv ) );
        }
        else {
            trell_message_t* msg = msgr.m_shmem_ptr;
            msg->m_type = TRELL_MESSAGE_GET_SCRIPTS;

            mrv = messenger_post( &msgr, TRELL_MESSAGE_SCRIPT_SIZE );
            if( mrv != MESSENGER_OK ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "mod_trell: messenger_post('%s') failed: %s",
                               dispatch_info->m_jobid, messenger_strerror( mrv ) );
                retval = HTTP_INTERNAL_SERVER_ERROR;
            }
            else if( msg->m_type == TRELL_MESSAGE_SCRIPT ) {
                retval = trell_send_script( sconf, r,
                                            msg->m_script.m_script, msg->m_size );
            }
            else {
                retval = HTTP_INTERNAL_SERVER_ERROR;
            }

            mrv = messenger_unlock( &msgr );
            if( mrv != MESSENGER_OK ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "mod_trell: messenger_unlock('%s') failed: %s",
                               dispatch_info->m_jobid, messenger_strerror( mrv ) );
            }
        }
        mrv = messenger_free( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_free('%s') failed: %s",
                           dispatch_info->m_jobid,
                           messenger_strerror( mrv ) );
        }
    }
    return retval;
}

int
trell_handle_get_renderlist( trell_sconf_t*          sconf,
                             request_rec*            r,
                             trell_dispatch_info_t*  dispatch_info )
{


    int retval = HTTP_INTERNAL_SERVER_ERROR;

    messenger_t msgr;
    messenger_status_t mrv;

    mrv = messenger_init( &msgr, dispatch_info->m_jobid );
    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_init('%s') failed: %s",
                       dispatch_info->m_jobid, messenger_strerror( mrv ) );
        return HTTP_NOT_FOUND;
    }
    else {
        mrv = messenger_lock( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_lock('%s') failed: %s",
                           dispatch_info->m_jobid, messenger_strerror( mrv ) );
        }
        else {
            trell_message_t* msg = msgr.m_shmem_ptr;
            msg->m_type = TRELL_MESSAGE_GET_RENDERLIST;
            memcpy( msg->m_get_renderlist.m_session_id, dispatch_info->m_sessionid, TRELL_SESSIONID_MAXLENGTH );
            memcpy( msg->m_get_renderlist.m_key, dispatch_info->m_key, TRELL_KEYID_MAXLENGTH );
            memcpy( msg->m_get_renderlist.m_timestamp, dispatch_info->m_timestamp, TRELL_TIMESTAMP_MAXLENGTH );

            mrv = messenger_post( &msgr, TRELL_MESSAGE_GET_RENDERLIST_SIZE );
            if( mrv != MESSENGER_OK ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "mod_trell: messenger_post('%s') failed: %s",
                               dispatch_info->m_jobid, messenger_strerror( mrv ) );
                retval = HTTP_INTERNAL_SERVER_ERROR;
            }
            else if( msg->m_type == TRELL_MESSAGE_XML ) {
                retval = trell_send_xml( sconf, r,
                                         msg->m_xml.m_xml, msg->m_size );
            }
            else {
                retval = HTTP_INTERNAL_SERVER_ERROR;
            }

            mrv = messenger_unlock( &msgr );
            if( mrv != MESSENGER_OK ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "mod_trell: messenger_unlock('%s') failed: %s",
                               dispatch_info->m_jobid, messenger_strerror( mrv ) );
            }
        }
        mrv = messenger_free( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_free('%s') failed: %s",
                           dispatch_info->m_jobid,
                           messenger_strerror( mrv ) );
        }
    }
    return retval;
}

int
trell_handle_get_snapshot( trell_sconf_t*          sconf,
                           request_rec*            r,
                           trell_dispatch_info_t*  dispatch_info )
{
    int retval = HTTP_INTERNAL_SERVER_ERROR;

    messenger_t msgr;
    messenger_status_t mrv;

    if( ( dispatch_info->m_width < 1     ) ||
        ( dispatch_info->m_width > 2048  ) ||
        ( dispatch_info->m_width < 1    ) ||
        ( dispatch_info->m_height > 2048 ) )
    {
        return HTTP_INSUFFICIENT_STORAGE;
    }

    // open connection to messenger
    mrv = messenger_init( &msgr, dispatch_info->m_jobid );
    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_init('%s') failed: %s",
                       dispatch_info->m_jobid, messenger_strerror( mrv ) );
        return HTTP_NOT_FOUND;
    }
    else {
        // try to get a lock
        mrv = messenger_lock( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_lock('%s') failed: %s",
                           dispatch_info->m_jobid, messenger_strerror( mrv ) );
        }
        else {
            // create query message
            trell_message_t* msg = msgr.m_shmem_ptr;
            msg->m_type = TRELL_MESSAGE_GET_SNAPSHOT;
            msg->m_get_snapshot.m_pixel_format = TRELL_PIXEL_FORMAT_BGR8;
            msg->m_get_snapshot.m_width = dispatch_info->m_width;
            msg->m_get_snapshot.m_height = dispatch_info->m_height;
            memcpy( msg->m_get_snapshot.m_session_id, dispatch_info->m_sessionid, TRELL_SESSIONID_MAXLENGTH );
            memcpy( msg->m_get_snapshot.m_key, dispatch_info->m_key, TRELL_KEYID_MAXLENGTH );

            // post query
            mrv = messenger_post( &msgr, TRELL_MESSAGE_GET_POLICY_UPDATE_SIZE );

            // failed to post query
            if( mrv != MESSENGER_OK ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "mod_trell: messenger_post('%s') failed: %s",
                               dispatch_info->m_jobid, messenger_strerror( mrv ) );
                retval = HTTP_INTERNAL_SERVER_ERROR;
                mrv = messenger_unlock( &msgr );
            }
            else if( msg->m_type == TRELL_MESSAGE_IMAGE ) {

                if( dispatch_info->m_request == TRELL_REQUEST_PNG ) {
                    retval = trell_send_png( sconf, r, dispatch_info,
                                             msg->m_image.m_pixel_format,
                                             msg->m_image.m_width,
                                             msg->m_image.m_height,
                                             msg->m_image.m_data,
                                             0u,
                                             &msgr,
                                             &mrv );
                    // messenger is unlock by send_png
                    // mrv = messenger_unlock( &msgr );
                }
                else {
                    retval = HTTP_NOT_IMPLEMENTED;
                    mrv = messenger_unlock( &msgr );
                }
            }
            else {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "mod_trell: argh" );
                mrv = messenger_unlock( &msgr );
            }

            if( mrv != MESSENGER_OK ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "mod_trell: messenger_unlock('%s') failed: %s",
                               dispatch_info->m_jobid, messenger_strerror( mrv ) );
            }
        }
        mrv = messenger_free( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_free('%s') failed: %s",
                           dispatch_info->m_jobid,
                           messenger_strerror( mrv ) );
        }
    }
    return retval;
}


int
trell_handle_get_model_update( trell_sconf_t* sconf,
                                request_rec* r,
                                trell_dispatch_info_t*  dispatch_info )
{

    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                   "mod_trell: has revision = %d", dispatch_info->m_revision );

    messenger_t msgr;
    messenger_status_t mrv;

    // open connection to messenger
    mrv = messenger_init( &msgr, dispatch_info->m_jobid );
    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_init('%s') failed: %s",
                       dispatch_info->m_jobid, messenger_strerror( mrv ) );
        return HTTP_NOT_FOUND;
    }
    else {
        // Create timeout for request
        struct timespec timeout;
        clock_gettime( CLOCK_REALTIME, &timeout );
        timeout.tv_sec += 30;

        int retval = HTTP_REQUEST_TIME_OUT;

        int i;

        for( i=0; (i<15) && (retval == HTTP_REQUEST_TIME_OUT); i++) {

            // try to get a lock
            mrv = messenger_lock( &msgr );
            if( mrv != MESSENGER_OK ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "mod_trell: messenger_lock('%s') failed: %s",
                               dispatch_info->m_jobid, messenger_strerror( mrv ) );
            }
            else {

                // create query message
                trell_message_t* msg = msgr.m_shmem_ptr;
                msg->m_type = TRELL_MESSAGE_GET_POLICY_UPDATE;
                msg->m_size = 0u;
                msg->m_get_model_update_payload.m_revision = dispatch_info->m_revision;
                strncpy( msg->m_get_model_update_payload.m_session_id,
                         dispatch_info->m_sessionid,
                         TRELL_SESSIONID_MAXLENGTH );
                msg->m_get_model_update_payload.m_session_id[ TRELL_SESSIONID_MAXLENGTH-1 ] = '\0';

                // post query
                mrv = messenger_post( &msgr, TRELL_MESSAGE_GET_POLICY_UPDATE_SIZE );

                // failed to post query
                if( mrv != MESSENGER_OK ) {
                    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                                   "mod_trell: messenger_post('%s') failed: %s",
                                   dispatch_info->m_jobid, messenger_strerror( mrv ) );
                    retval = HTTP_INTERNAL_SERVER_ERROR;
                }

                // we got an update
                else if( msg->m_type == TRELL_MESSAGE_XML ) {
                    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "XML size=%d", (int)msg->m_size );

                    // new state
                    retval = trell_send_xml( sconf,
                                                    r,
                                                    msg->m_xml_payload,
                                                    msg->m_size );
                    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "longpoll returns %d (OK=%d)", retval, OK );
                }

                // release lock
                mrv = messenger_unlock( &msgr );
                if( mrv != MESSENGER_OK ) {
                    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                                   "mod_trell: messenger_unlock('%s') failed: %s",
                                   dispatch_info->m_jobid, messenger_strerror( mrv ) );
                }
            }

            if( retval != HTTP_REQUEST_TIME_OUT ) {
                // We have a response!
            }
            else {
                // Check if we want to hang around for until something happens.
                struct timespec current;
                clock_gettime( CLOCK_REALTIME, &current );
                if( current.tv_sec >= timeout.tv_sec ) {
                    ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "mod_trell: request timer timed out." );
                    break;
                }
                else {
                    mrv = messenger_wait_for_notification( &msgr, 10 );
                    switch( mrv ) {
                    case MESSENGER_OK:
                    case MESSENGER_TIMEOUT:
                        // Expected behaviour
                        break;
                    default:
                        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r,
                                       "mod_trell: messenger_wait_for_notification: %s",
                                       messenger_strerror( mrv ) );
                        break;
                    }
                }
            }
        }

        mrv = messenger_free( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_free('%s') failed: %s",
                           dispatch_info->m_jobid,
                           messenger_strerror( mrv ) );
        }

        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: returned %d", retval );
        return retval;
    }
}

int
trell_handle_update_state( trell_sconf_t* sconf,
                           request_rec* r,
                           trell_dispatch_info_t*  dispatch_info )
{

    // check method
    if( r->method_number != M_POST ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: method != POST" );
        return HTTP_METHOD_NOT_ALLOWED;
    }
    // check if content type is correct
    const char* content_type = apr_table_get( r->headers_in, "Content-Type" );
    if( content_type == NULL ) {
        ap_log_rerror(  APLOG_MARK, APLOG_ERR, 0, r, "mod_trell: No content type" );
        return HTTP_BAD_REQUEST;
    }

    // In case we get Content-Type: text/xml; encoding=foo.
    char* semicolon = strchr( content_type, ';' );
    if( semicolon != NULL ) {
        // Chop line at semicolon.
        *semicolon = '\0';
    }
    if( ! ( ( strcasecmp( "application/xml", content_type) == 0 ) ||
            ( strcasecmp( "text/xml", content_type ) == 0 ) ) )
    {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: Unsupported content-type '%s'", content_type );
        return HTTP_BAD_REQUEST;
    }

    int retval = HTTP_NOT_IMPLEMENTED;


    messenger_t msgr;
    messenger_status_t mrv;


    // open connection to messenger
    mrv = messenger_init( &msgr, dispatch_info->m_jobid );
    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_init('%s') failed: %s",
                       dispatch_info->m_jobid, messenger_strerror( mrv ) );
        retval = HTTP_NOT_FOUND;
    }
    else {
        // try to get a lock
        mrv = messenger_lock( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_lock('%s') failed: %s",
                           dispatch_info->m_jobid, messenger_strerror( mrv ) );
            retval = HTTP_INTERNAL_SERVER_ERROR;
        }
        else {

            // Create message
            trell_message_t* msg = msgr.m_shmem_ptr;
            msg->m_type = TRELL_MESSAGE_UPDATE_STATE;
            msg->m_size = 0u;

            // copy input request into shmem

            // set up fetching of input
            apr_bucket_brigade* bb;
            apr_bucket* b;
            apr_status_t rv;

            bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );

            int keep_going = 1;
            while(keep_going == 1) {
                apr_size_t free = msgr.m_shmem_size - msg->m_size - TRELL_MSGHDR_SIZE - 1;
                rv = ap_get_brigade( r->input_filters,
                                     bb,
                                     AP_MODE_READBYTES,
                                     APR_BLOCK_READ,
                                     free );
                if( rv != APR_SUCCESS ) {
                    retval = HTTP_INTERNAL_SERVER_ERROR;
                    break;
                }
                for( b = APR_BRIGADE_FIRST(bb); b!=APR_BRIGADE_SENTINEL(bb); b = APR_BUCKET_NEXT(b) ) {
                    if( APR_BUCKET_IS_EOS(b) ) {
                        keep_going = 0;
                        break;
                    }
                    else if( APR_BUCKET_IS_METADATA(b) ) {
                        continue;
                    }

                    apr_size_t bytes = 0;
                    const char* buf = NULL;
                    rv = apr_bucket_read( b,
                                          &buf,
                                          &bytes,
                                          APR_BLOCK_READ );
                    if( rv != APR_SUCCESS ) {
                        keep_going = 0;
                        retval = HTTP_INTERNAL_SERVER_ERROR;
                        break;
                    }
                    if( free < bytes ) {
                        keep_going = 0;
                        retval = HTTP_INSUFFICIENT_STORAGE;
                        break;
                    }

                      memcpy( msg->m_update_state.m_xml + msg->m_size, buf, bytes );
                    msg->m_size += bytes;
                }
            }
            apr_brigade_cleanup( bb );

            // post query
            mrv = messenger_post( &msgr, TRELL_MESSAGE_UPDATE_STATE );

            // failed to post query
            if( mrv != MESSENGER_OK ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "mod_trell: messenger_post('%s') failed: %s",
                               dispatch_info->m_jobid, messenger_strerror( mrv ) );
                retval = HTTP_INTERNAL_SERVER_ERROR;
            }

            // we got an update
            else if( msg->m_type == TRELL_MESSAGE_OK ) {
                retval = HTTP_NO_CONTENT;
            }
            else {
                retval = HTTP_INTERNAL_SERVER_ERROR;
            }


            // release lock
            mrv = messenger_unlock( &msgr );
            if( mrv != MESSENGER_OK ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "mod_trell: messenger_unlock('%s') failed: %s",
                               dispatch_info->m_jobid, messenger_strerror( mrv ) );
                retval = HTTP_INTERNAL_SERVER_ERROR;
            }
        }

        mrv = messenger_free( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_free('%s') failed: %s",
                           dispatch_info->m_jobid,
                           messenger_strerror( mrv ) );
            retval = HTTP_INTERNAL_SERVER_ERROR;
        }
    }

    return retval;
}





int
trell_job_rpc_handle( trell_sconf_t* sconf,
                      request_rec*r,
                      xmlSchemaPtr schema,
                      const char* job )
{

    apr_array_header_t* brigades = apr_array_make( r->pool, 10, sizeof(apr_bucket_brigade*) );
    if( strcmp( job, sconf->m_master_id ) ) {

        int ret = trell_parse_xml( sconf, r, brigades, schema );
        if( ret != OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: Failure to get XML content" );
            return ret;
        }
    }
    else {

        int ret = trell_parse_xml( sconf, r, brigades, schema );
        if( ret != OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: Failure to get XML content" );
            return ret;
        }
    }


    struct messenger msgr;
    messenger_status_t mrv;

    mrv = messenger_init( &msgr, job );
    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_init('%s') failed: %s",  job, messenger_strerror( mrv ) );
        return HTTP_NOT_FOUND;
    }

    mrv = messenger_lock( &msgr );
    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_lock('%s') failed: %s",  job, messenger_strerror( mrv ) );

        mrv = messenger_free( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_free('%s') failed: %s",
                           job,
                           messenger_strerror( mrv ) );
        }
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    struct trell_message* msg = msgr.m_shmem_ptr;

    size_t size = trell_flatten_brigades_into_mem( sconf, r,
                                                   msg->m_xml_payload,
                                                   msgr.m_shmem_size - TRELL_MSGHDR_SIZE,
                                                   brigades );
    if( size == 0 ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: Failed to flatten brigades" );
        mrv = messenger_unlock( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_unlock('%s') failed: %s",  job, messenger_strerror( mrv ) );
        }
        mrv = messenger_free( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_free('%s') failed: %s",  job, messenger_strerror( mrv ) );
         }
        return HTTP_INTERNAL_SERVER_ERROR;
    }


    // --- post message ---
    msg->m_type = TRELL_MESSAGE_XML;
    msg->m_size = size;

    mrv = messenger_post( &msgr, size + TRELL_MSGHDR_SIZE );

    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_post('%s') failed: %s",  job, messenger_strerror( mrv ) );
        mrv = messenger_unlock( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_unlock('%s') failed: %s",  job, messenger_strerror( mrv ) );
        }
        mrv = messenger_free( &msgr );
        if( mrv != MESSENGER_OK ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "mod_trell: messenger_free('%s') failed: %s",  job, messenger_strerror( mrv ) );
         }
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    // --- send reply to client ---
    int retcode = trell_send_reply_xml( sconf, r, &msgr );
    mrv = messenger_unlock( &msgr );
    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_unlock('%s') failed: %s",  job, messenger_strerror( mrv ) );
    }
    mrv = messenger_free( &msgr );
    if( mrv != MESSENGER_OK ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: messenger_free('%s') failed: %s",  job, messenger_strerror( mrv ) );
    }
    return retcode;
}
