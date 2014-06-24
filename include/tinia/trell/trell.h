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

#define TRELL_SESSIONID_MAXLENGTH 64
#define TRELL_REQUESTNAME_MAXLENGTH 128
#define TRELL_KEYID_MAXLENGTH 64
#define TRELL_TIMESTAMP_MAXLENGTH 128

/** The pixel formats that is used in the trell system. */
enum TrellPixelFormat {
    /** 8-bit normalized bgr data. */
    TRELL_PIXEL_FORMAT_BGR8,
    /** 8-bit normalized bgr data + 24-bit fixed point depth. */
    TRELL_PIXEL_FORMAT_BGR8_CUSTOM_DEPTH // @@@
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

/** Base message struct.
 *
 * Container for:
 * - TRELL_MESSAGE_OK
 * - TRELL_MESSAGE_ERROR
 */
typedef struct tinia_msg
{
    enum TrellMessageType   type;    
} tinia_msg_t;


/** Message struct for TRELL_MESSAGE_HEARTBEAT. */
typedef struct tinia_msg_heartbeat
{
    tinia_msg_t         msg;
    enum TrellJobState  state;
    char                job_id[ TINIA_IPC_JOBID_MAXLENGTH+1 ];
} tinia_msg_heartbeat_t;


/** Message struct for TRELL_MESSAGE_HEARTBEAT. */
typedef struct {
    tinia_msg_t             msg;
    enum TrellPixelFormat   pixel_format;
    unsigned int            width;
    unsigned int            height;
    char                    session_id[TRELL_SESSIONID_MAXLENGTH + 1];
    char                    key[ TRELL_KEYID_MAXLENGTH + 1 ];
} tinia_msg_get_snapshot_t;

/** Message struct for TRELL_MESSAGE_GET_SCRIPTS. */
typedef struct {
    tinia_msg_t             msg;
} tinia_msg_get_script_t;

/** Message struct for TRELL_MESSAGE_SCRIPT. */
typedef struct {
    tinia_msg_t             msg;
} tinia_msg_script_t;

/** Message struct for TRELL_MESSAGE_UPDATE_STATE. */
typedef struct {
    tinia_msg_t             msg;
    char                    session_id[TRELL_SESSIONID_MAXLENGTH + 1];
} tinia_msg_update_exposed_model_t;


/** Message struct for TRELL_MESSAGE_GET_POLICY_UPDATE. */
typedef struct {
    tinia_msg_t             msg;
    unsigned int            revision;
    char                    session_id[TRELL_SESSIONID_MAXLENGTH + 1];
} tinia_msg_get_exposed_model_t;


/** Message struct for msg.type=TRELL_MESSAGE_GET_RENDERLIST. */
typedef struct {
    tinia_msg_t             msg;
    char                    session_id[TRELL_SESSIONID_MAXLENGTH + 1 ];
    char                    key[ TRELL_KEYID_MAXLENGTH + 1 ];
    char                    timestamp[ TRELL_TIMESTAMP_MAXLENGTH + 1 ];
} tinia_msg_get_renderlist_t;


/** Message struct for TRELL_MESSAGE_IMAGE. */
typedef struct {
    tinia_msg_t             msg;
    enum TrellPixelFormat   pixel_format;
    unsigned int            width;
    unsigned int            height;
} tinia_msg_image_t;




/** Message struct for TRELL_MESSAGE_XML. */
typedef struct {
    tinia_msg_t             msg;
} tinia_msg_xml_t;



#ifdef __cplusplus
}
#endif
#endif // TRELL_H
