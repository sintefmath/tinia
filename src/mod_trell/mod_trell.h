#ifndef MOD_TRELL_H
#define MOD_TRELL_H

#include <httpd.h>
#include <apr_hash.h>
#include <libxml/xmlschemas.h>
#include "trell/messenger.h"
#include "trell/trell.h"



/** Trell configuration structure.
  *
  * Elements set in httpd.conf.
  */
typedef struct mod_trell_svr_conf
{
    /** Flag used to informed that this struct contains sane values. */
    int           m_well_formed;
    /** Use TrellMasterId to set. */
    const char*   m_master_id;
    /** Use TrellMasterExe to set. */
    const char*   m_master_exe;
    /** Use TrellAppRoot to set. */
    const char*   m_app_root_dir;

    /** Root directory of schemas, use TrellSchemaRoot to set. */
    const char*   m_schema_root_dir;

    /** WWW root for jobs, use TrellJobWWWRoot to set. */
    const char*   m_job_www_root;

    /** Schema that validates XML RPC queries to /ops */
    xmlSchemaPtr  m_rpc_ops_schema;
    /** Schema that validates XML RPC queries to /master */
    xmlSchemaPtr  m_rpc_master_schema;
    /** Schema that validates XML RPC queries to /job */
    xmlSchemaPtr  m_rpc_job_schema;
    /** Schema that validates XML RPC replies */
    xmlSchemaPtr  m_rpc_reply_schema;
} trell_sconf_t;

enum TrellComponent {
    TRELL_COMPONENT_NONE = 0,
    TRELL_COMPONENT_OPS,
    TRELL_COMPONENT_MASTER,
    TRELL_COMPONENT_JOB
};

enum TrellRequest {
    TRELL_REQUEST_NONE = 0,
    TRELL_REQUEST_STATIC_FILE,
    TRELL_REQUEST_RPC_XML,
    TRELL_REQUEST_POLICY_UPDATE_XML,
    TRELL_REQUEST_STATE_UPDATE_XML,
    TRELL_REQUEST_PNG,
    TRELL_REQUEST_GET_RENDERLIST
};

enum TrellModAction {
    TRELL_MOD_ACTION_NONE,
    TRELL_MOD_ACTION_RESTART_MASTER
};

typedef struct mod_trell_dispatch_info
{
    enum TrellComponent  m_component;
    enum TrellRequest    m_request;
    enum TrellModAction  m_mod_action;
    int                  m_revision;
    /** Send response as a base64 encoded string. */
    int                  m_base64;
    int                  m_width;
    int                  m_height;
    char                 m_jobid[TRELL_JOBID_MAXLENGTH];
    char                 m_sessionid[TRELL_SESSIONID_MAXLENGTH];
    char                 m_requestname[TRELL_REQUESTNAME_MAXLENGTH];
    char                 m_key[TRELL_KEYID_MAXLENGTH];
    char                 m_timestamp[ TRELL_TIMESTAMP_MAXLENGTH ];
} trell_dispatch_info_t;

int
trell_decode_path_info( trell_dispatch_info_t* dispatch_info,
                        request_rec*           r );

/** Handles long-polling request from client.
  *
  * - Opens connection to job, if fails, returns HTTP_NOT_FOUND.
  * - Set timeout to current time + 30 seconds.
  * - For 15 iterations:
  * -- Check for an update, if there is one, return it.
  * -- If no update, wait for notification for 10 seconds.
  *
  */
int
trell_handle_get_policy_update( trell_sconf_t*          sconf,
                                request_rec*            r,
                                trell_dispatch_info_t*  dispatch_info );

/** Handles a policy update from client.
  *
  * - Checks if method is post, if not return HTTP_METHOD_NOT_ALLOWED.
  * - Checks if for presence and value of content type, if fail return HTTP_BAD_REQUEST.
  * - Opens connection to job, if fails, return HTTP_NOT_FOUND.
  * - Lock connection to job, if fails, return HTTP_INTERNAL_SERVER_ERROR.
  * - Create message with XML payload, send to job
  * -- If apache reading fails, return HTTP_INTERNAL_SERVER_ERROR.
  * -- If messenger buffer is too small, return HTTP_INSUFFICIENT_STORAGE.
  * -- If message post fails, return HTTP_INTERNAL_SERVER.
  * - Unlock and close connection to job, if fails return HTTP_INTERNAL_SERVER_ERROR.
  * - If succeeds, return HTTP_NO_CONTENT.
  */
int
trell_handle_update_state( trell_sconf_t*          sconf,
                           request_rec*            r,
                           trell_dispatch_info_t*  dispatch_info );

int
trell_handle_get_snapshot( trell_sconf_t*          sconf,
                           request_rec*            r,
                           trell_dispatch_info_t*  dispatch_info );

int
trell_handle_get_renderlist( trell_sconf_t*          sconf,
                             request_rec*            r,
                             trell_dispatch_info_t*  dispatch_info );

int
trell_send_xml( trell_sconf_t*   sconf,
                request_rec*     r,
                const char*      payload,
                const size_t     payload_size );

int
trell_send_xml_success( trell_sconf_t*  sconf,
                        request_rec*    r );

int
trell_send_xml_failure( trell_sconf_t*  sconf,
                        request_rec*    r );

int
trell_send_png( trell_sconf_t*          sconf,
                request_rec*            r,
                trell_dispatch_info_t*  dispatch_info,
                enum TrellPixelFormat   format,
                const int               width,
                const int               height,
                const char*             payload,
                const size_t            payload_size );
int
trell_send_reply_static_file( trell_sconf_t* sconf,
                       request_rec*   r,
                       trell_dispatch_info_t* dinfo );



/** Send the XML contents of a locked messenger back to the HTTP client.
  *
  * \param r     The request structure of the request.
  * \param msgr  A locked messenger containing the reply to encode. The
  *              The messenger will not be unlocked.
  * \returns     The return value for the handler.
  */
int
trell_send_reply_xml( trell_sconf_t* sconf, request_rec* r, struct messenger* msgr );





/** Handle an RPC request that is directed to mod_trell.
  *
  * \param sconf  The server configuration.
  * \param r      The request structure of the request.
  * \returns      The return value for the operation.
  */
int
trell_ops_rpc_handle( trell_sconf_t* sconf, request_rec* r );


/** Restart the master. */
int
trell_ops_do_restart_master( trell_sconf_t* sconf, request_rec*r );


/** Handle an RPC request that is directed to a job (i.e. passed over IPC).
  *
  * \param sconf  The server configuration.
  * \param r      The request structure of the request.
  * \param job    The id of the job that should process the request.
  * \returns      The return value for the operation.
  */
int
trell_job_rpc_handle( trell_sconf_t* sconf, request_rec*r, xmlSchemaPtr schema, const char* job );


apr_hash_t*
trell_parse_args_uniq_key( request_rec* r, char* args );


/** Checks if a job-name is valid
  *
  * \returns 1 If the job-name is valid and 0 otherwise.
  */
int
trell_valid_jobid( trell_sconf_t* sconf, request_rec* r, const char* job );

/** Flattens a list of brigades into a memory chunk.
  *
  * \param sconf        The server configuration.
  * \param r            The request structure of the request.
  * \param buffer       The memory chunk into where to store the contents of the
  *                     brigades.
  * \param buffer_size  The size of the memory chunk, in bytes.
  * \param brigades     An array of pointers to brigades.
  */
size_t
trell_flatten_brigades_into_mem( trell_sconf_t* sconf, request_rec*r,
                                 char* buffer, size_t buffer_size,
                                 apr_array_header_t* brigades );

int
trell_parse_xml( trell_sconf_t* sconf,
                 request_rec* r,
                 apr_array_header_t* brigades,
                 xmlSchemaPtr schema );


/** LibXML2 error callback to use when we have a server_rec. */
void
trell_xml_error_s_cb( void* ctx, const char* msg, ... );



struct TrellLibXMLState
{
    void*                m_original_error_context;
    xmlGenericErrorFunc  m_original_error_func;
};

void
trell_libxml_state_set_r( struct TrellLibXMLState* old, request_rec* r );

void
trell_libxml_state_restore( struct TrellLibXMLState* old );


#endif // MOD_TRELL_H
