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

#ifndef TRELL_H
#define TRELL_H

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

#define TRELL_JOBID_MAXLENGTH 64
#define TRELL_SESSIONID_MAXLENGTH 64
#define TRELL_REQUESTNAME_MAXLENGTH 128
#define TRELL_KEYID_MAXLENGTH 64
#define TRELL_TIMESTAMP_MAXLENGTH 128

/** The pixel formats that is used in the trell system. */
enum TrellPixelFormat {
    /** 8-bit normalized bgr data. */
    TRELL_PIXEL_FORMAT_BGR8
};

/** States that a MessageBox/Master/Job/InteractiveJob can be in */
enum TrellJobState {
    /** Process has begun to be set up. */
    TRELL_JOBSTATE_NOT_STARTED,
    /** Process has succesfully initialized and runs. */
    TRELL_JOBSTATE_RUNNING,
    /** Process has finished its job successfully, but is still not dead. */
    TRELL_JOBSTATE_FINISHED,
    /** Process has failed somehow, but is still not dead. */
    TRELL_JOBSTATE_FAILED,
    /** A successful process is dead. */
    TRELL_JOBSTATE_TERMINATED_SUCCESSFULLY,
    /** An unsuccessful process is dead. */
    TRELL_JOBSTATE_TERMINATED_UNSUCCESSFULLY
};

/** Types of messages that is passed between trell process. */
enum TrellMessageType {
    /** Replies that an error has occured, no payload. */
    TRELL_MESSAGE_ERROR,
    /** Replies that the operation went OK, no payload. */
    TRELL_MESSAGE_OK,
    /** Query or reply that contains an xml payload. */
    TRELL_MESSAGE_XML,

    /** Reply that contains JavaScript */
    TRELL_MESSAGE_SCRIPT,

    /** A heartbeat query. */
    TRELL_MESSAGE_HEARTBEAT,
    /** A suggestion for the job to die. */
    TRELL_MESSAGE_DIE,

    /** Message to query for a model. */
    TRELL_MESSAGE_GET_POLICY_UPDATE,

    /** Message with an updated state. */
    TRELL_MESSAGE_UPDATE_STATE,

    /** Message to query for an image. */
    TRELL_MESSAGE_GET_SNAPSHOT,


    /** Message contains an unencoded image. */
    TRELL_MESSAGE_IMAGE,

    TRELL_MESSAGE_GET_RENDERLIST,

    TRELL_MESSAGE_GET_SCRIPTS
};

typedef struct trell_message
{
    /** The type of the message. */
    enum TrellMessageType       m_type;
    /** The size of the payload. */
    size_t                      m_size;
    union {

        struct {
            unsigned int            m_revision;
            char                    m_session_id[TRELL_SESSIONID_MAXLENGTH];
            char                    m_tail;
        }                       m_get_model_update_payload;

        struct {
            char                    m_xml[1];
        }                       m_xml;

        struct {
            char m_script[1];
        } m_script;

        struct {
            char                    m_xml[1];
        }                       m_update_state;

        struct {
            enum TrellPixelFormat   m_pixel_format;
            unsigned int            m_width;
            unsigned int            m_height;
            char                    m_session_id[TRELL_SESSIONID_MAXLENGTH];
            char                    m_key[ TRELL_KEYID_MAXLENGTH ];
            char                    m_tail;
        }                       m_get_snapshot;

        struct {
            enum TrellPixelFormat   m_pixel_format;
            unsigned int            m_width;
            unsigned int            m_height;
            char                    m_tail;
            char                    m_data[1];
        }                       m_image;

        struct {
            char                    m_session_id[TRELL_SESSIONID_MAXLENGTH];
            char                    m_key[ TRELL_KEYID_MAXLENGTH ];
            char                    m_timestamp[ TRELL_TIMESTAMP_MAXLENGTH ];
            char                    m_tail;

        }                       m_get_renderlist;


        /** Label used for address calculations. */
        char                    m_payload;
        /** Payload for character-based messages (XML). */
        char                    m_xml_payload[1];
        /** Payload for heartbeat messages. */
        struct {
            /** State of job sending the heartbeat. */
            enum TrellJobState  m_state;
            /** Id of job sending the heartbeat. */
            char                m_job_id[1];
        }                       m_ping_payload;
    };
} trell_message_t;

#define TRELL_MSGHDR_SIZE                       (offsetof(trell_message_t, m_payload))
#define TRELL_MESSAGE_GET_POLICY_UPDATE_SIZE    (offsetof(trell_message_t, m_get_model_update_payload.m_tail ) )
#define TRELL_MESSAGE_IMAGE_SIZE                (offsetof(trell_message_t, m_image.m_data))
#define TRELL_MESSAGE_GET_RENDERLIST_SIZE       (offsetof(trell_message_t, m_get_renderlist.m_tail ) )
#define TRELL_MESSAGE_XML_SIZE                  (offsetof(trell_message_t, m_xml.m_xml ))
#define TRELL_MESSAGE_SCRIPT_SIZE               (offsetof(trell_message_t, m_script.m_script ))

#ifdef __cplusplus
}
#endif
#endif // TRELL_H
