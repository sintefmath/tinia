
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

#if 0

    // --- Sanity checks ---

    // We only allow post requests
    if( r->method_number != M_POST ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: Method is not POST" );
        return HTTP_METHOD_NOT_ALLOWED;
    }

    // And we require a XML document
    const char* content_type = apr_table_get( r->headers_in, "Content-Type" );
    if( content_type == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "mod_tell: No content type" );
        return HTTP_BAD_REQUEST;
    }
    if( ! ( ( strcasecmp( "application/xml", content_type) == 0 ) ||
            ( strcasecmp( "text/xml", content_type ) == 0 ) ) )
    {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: Unsupported content-type '%s'", content_type );
        return HTTP_BAD_REQUEST;
    }

    // --- set up messenger ---
    struct messenger msgr;
    if( messenger_init( &msgr, job ) < 1 ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, OK, r,
                       "mod_trell: Failed to open messenger to '%s'",  job );
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    if( messenger_lock( &msgr ) < 1 ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, OK, r,
                       "mod_trell: Failed to lock messenger to '%s'",  job );
        messenger_free( &msgr );
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    struct trell_message* msg = msgr.m_shmem_ptr;

    // --- fetch data from client ---
    struct apr_bucket_brigade* b = apr_brigade_create( r->pool, r->connection->bucket_alloc );
    apr_status_t status = APR_SUCCESS;
    apr_size_t read = 0;
    size_t offset = 0;
    do {
        read = msgr.m_shmem_size - offset - TRELL_MSGHDR_SIZE;
        status = ap_get_brigade( r->input_filters,
                                 b,
                                 AP_MODE_READBYTES,
                                 APR_BLOCK_READ,
                                 read );
        apr_brigade_flatten( b, msg->m_xml_payload + offset, &read );
        apr_brigade_cleanup( b );

        offset += read;
    }
    while( status == APR_SUCCESS && read > 0 );
    apr_brigade_destroy( b );

    if( status != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, status, r,
                       "mod_trell: Error reading POST data" );
        messenger_unlock( &msgr );
        messenger_free( &msgr );
        return HTTP_INTERNAL_SERVER_ERROR;
    }


    // --- post message ---
    msg->m_type = TRELL_MESSAGE_XML;
    msg->m_size = offset;
    if( messenger_post( &msgr, offset + TRELL_MSGHDR_SIZE ) < 1 ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, OK, r,
                       "mod_trell: Failed to post through messenger to '%s'",  job );
        messenger_unlock( &msgr );
        messenger_free( &msgr );
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    // --- send reply to client ---
    int retcode = trell_send_reply_xml( r, &msgr );
    messenger_unlock( &msgr );
    messenger_free( &msgr );
    return retcode;
#endif
}


int
trell_send_xml( trell_sconf_t*   sconf,
                       xmlSchemaPtr     schema,
                       request_rec*     r,
                       const char*      payload,
                       const size_t     payload_size )
{


    size_t size = payload_size;
    if( size > 0 && payload[size] == '\0' ) {
        ap_log_rerror( APLOG_MARK, APLOG_NOTICE, 0, r, "Trimmed tailing zero from XML." );
        size = size-1;
    }

    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r, "XML size=%d", (int)size );

#ifdef TRELL_VALIDATE_XML_REPLY
    if( schema != NULL ) {
        struct TrellLibXMLState libxml_state;
        trell_libxml_state_set_r( &libxml_state, r );

        xmlSchemaValidCtxtPtr schema_ctx = xmlSchemaNewValidCtxt( sconf->m_rpc_reply_schema );
        if( schema_ctx == NULL ) {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                           "Failed to create validation context for reply schema" );
        }
        else {
            xmlDocPtr doc = xmlReadMemory( payload, size,
                                           "cloudviz.sintef.no/",
                                           XML_CHAR_ENCODING_NONE,
                                           0 );
            if( doc == NULL ) {
                ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                               "mod_trell: Failed to parse the reply XML" );
            }
            else {
                if( xmlSchemaValidateDoc( schema_ctx, doc ) != 0 ) {
                    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                                   "mod_trell: Reply XML is NOT valid" );
                }
                xmlFreeDoc( doc );
            }
            xmlSchemaFreeValidCtxt( schema_ctx );
        }
        trell_libxml_state_restore( &libxml_state );
    }
#endif


    apr_bucket_brigade* bb;
    apr_bucket* b;
    char* datestring = apr_palloc( r->pool, APR_RFC822_DATE_LEN );
    apr_rfc822_date( datestring, apr_time_now() );

    ap_set_content_type( r, "application/xml" );
    ap_set_content_length( r, size );
    apr_table_setn( r->headers_out, "Last-Modified", datestring );

    bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );
    b = apr_bucket_transient_create( payload, size, bb->bucket_alloc );

    APR_BRIGADE_INSERT_TAIL( bb, b );
    APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );

    apr_status_t rv = ap_pass_brigade( r->output_filters, bb );
    if( rv != APR_SUCCESS ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Output error" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    else {
        return OK;
    }
}


int
trell_send_reply_xml( trell_sconf_t* sconf, request_rec* r, struct messenger* msgr )
{
    struct trell_message* msg = msgr->m_shmem_ptr;
    enum TrellMessageType message_type = msg->m_type;

    int retval = OK;
    if( message_type == TRELL_MESSAGE_XML ) {
        if( msg->m_size > 0 ) {

            // Check if we have to validate the result
            if( sconf->m_rpc_reply_schema != NULL ) {
                struct TrellLibXMLState libxml_state;
                trell_libxml_state_set_r( &libxml_state, r );

                xmlSchemaValidCtxtPtr schema_ctx = xmlSchemaNewValidCtxt( sconf->m_rpc_reply_schema );
                if( schema_ctx == NULL ) {
                    ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                                   "Failed to create validation context for reply schema" );
                }
                else {
                    xmlDocPtr doc = xmlReadMemory( msg->m_xml_payload, msg->m_size-1,
                                                   "cloudviz.sintef.no/",
                                                   XML_CHAR_ENCODING_NONE,
                                                   0 );
                    if( doc == NULL ) {
                        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                                       "mod_trell: Failed to parse the reply XML" );
                    }
                    else {
                        if( xmlSchemaValidateDoc( schema_ctx, doc ) != 0 ) {
                            ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                                           "mod_trell: Reply XML is NOT valid" );
                            retval = HTTP_INTERNAL_SERVER_ERROR;
                        }
                        xmlFreeDoc( doc );
                    }
                    xmlSchemaFreeValidCtxt( schema_ctx );
                }
                trell_libxml_state_restore( &libxml_state );
            }

            if( retval == OK ) {
                apr_bucket_brigade* bb;
                apr_bucket* b;
                char* datestring = apr_palloc( r->pool, APR_RFC822_DATE_LEN );
                apr_rfc822_date( datestring, apr_time_now() );

                ap_set_content_type( r, "application/xml" );
                ap_set_content_length( r, msg->m_size-1 ); // msg->m_size includes null terminator
                apr_table_setn( r->headers_out, "Last-Modified", datestring );

                bb = apr_brigade_create( r->pool, r->connection->bucket_alloc );
                b = apr_bucket_transient_create( msg->m_xml_payload, msg->m_size-1, bb->bucket_alloc );

                APR_BRIGADE_INSERT_TAIL( bb, b );
                APR_BRIGADE_INSERT_TAIL( bb, apr_bucket_eos_create( bb->bucket_alloc ) );

                apr_status_t rv = ap_pass_brigade( r->output_filters, bb );
                if( rv != APR_SUCCESS ) {
                    ap_log_rerror( APLOG_MARK, APLOG_ERR, rv, r, "Output error" );
                    retval = HTTP_INTERNAL_SERVER_ERROR;
                }
            }
        }
        else {
            ap_log_rerror( APLOG_MARK, APLOG_ERR, OK, r,
                           "mod_trell: No path for message type %d", message_type );
            retval = HTTP_NO_CONTENT;
        }
        return retval;
    }
    else {

    }
    return HTTP_INTERNAL_SERVER_ERROR;
}


int
trell_job_bmp_handle( trell_sconf_t* sconf, request_rec* r, apr_hash_t* args, const char* job )
{
    return HTTP_INTERNAL_SERVER_ERROR;
/*
    int width = 0;
    int height = 0;
    const char* ws = apr_hash_get( args, "width", APR_HASH_KEY_STRING );
    if( ws != NULL ) {
        width = atoi( ws );
    }
    const char* hs = apr_hash_get( args, "height", APR_HASH_KEY_STRING );
    if( hs != NULL ) {
        height = atoi( hs );
    }
    if( (width < 1) || (2000 <= width) || (height < 1) || (2000 <= height ) ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: Illegal image size %d x %d", width, height );
        return HTTP_BAD_REQUEST;
    }

    // --- set up messenger ---
    // currently, we just fake the message

    struct messenger msgr;
    msgr.m_shmem_size = sizeof(struct trell_message) + width*height*3;
    msgr.m_shmem_ptr = apr_palloc( r->pool, msgr.m_shmem_size );
    if( msgr.m_shmem_ptr == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: Failure to allocate memory for dummy image" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    struct trell_message* msg = msgr.m_shmem_ptr;
    msg->m_type = TRELL_MESSAGE_IMAGE_BGR;
    msg->m_image_payload.m_width = width;
    msg->m_image_payload.m_height = height;
    int i, j;
    char* p = msg->m_image_payload.m_data;
    for( j=0; j<height; j++) {
        for( i=0; i<width; i++ ) {
            *p++ = (i*j)%255;
            *p++ = (j)%255;
            *p++ = (i)%255;
        }
    }

    int retval = trell_send_reply_bmp( r, &msgr );

    return retval;
*/
}


int
trell_job_png_handle( trell_sconf_t* sconf, request_rec* r, apr_hash_t* args, const char* job )
{
    return HTTP_INTERNAL_SERVER_ERROR;
/*
    int width = 0;
    int height = 0;
    const char* ws = apr_hash_get( args, "width", APR_HASH_KEY_STRING );
    if( ws != NULL ) {
        width = atoi( ws );
    }
    const char* hs = apr_hash_get( args, "height", APR_HASH_KEY_STRING );
    if( hs != NULL ) {
        height = atoi( hs );
    }
    if( (width < 1) || (2000 <= width) || (height < 1) || (2000 <= height ) ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: Illegal image size %d x %d", width, height );
        return HTTP_BAD_REQUEST;
    }


    // --- set up messenger ---
    // currently, we just fake the message

    struct messenger msgr;
    msgr.m_shmem_size = sizeof(struct trell_message) + width*height*3;
    msgr.m_shmem_ptr = apr_palloc( r->pool, msgr.m_shmem_size );
    if( msgr.m_shmem_ptr == NULL ) {
        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                       "mod_trell: Failure to allocate memory for dummy image" );
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    struct trell_message* msg = msgr.m_shmem_ptr;
    msg->m_type = TRELL_MESSAGE_IMAGE_BGR;
    msg->m_image_payload.m_width = width;
    msg->m_image_payload.m_height = height;
    int i, j;
    char* p = msg->m_image_payload.m_data;
    for( j=0; j<height; j++) {
        for( i=0; i<width; i++ ) {
            *p++ = (i*j)%255;
            *p++ = (j)%255;
            *p++ = (i)%255;
        }
    }

    int retval = trell_send_reply_png( r, &msgr );

    return retval;
*/
}


int
trell_decode_args( trell_dispatch_info_t* dispatch_info, request_rec*r )
{
    if( dispatch_info->m_component == TRELL_COMPONENT_OPS ) {
        if( dispatch_info->m_request == TRELL_REQUEST_RPC_XML ) {
            apr_hash_t* args = trell_parse_args_uniq_key( r, r->args );
            if( args != NULL ) {
                const char* action = apr_hash_get( args, "action", APR_HASH_KEY_STRING );
                if( action != NULL ) {
                    if( strcmp( action, "restart_master" ) == 0 ) {
                        dispatch_info->m_mod_action = TRELL_MOD_ACTION_RESTART_MASTER;
                    }
                    else {
                        ap_log_rerror( APLOG_MARK, APLOG_ERR, 0, r,
                                       "mod_trell: unknown mod action '%s'", action );
                    }
                }
            }
        }
    }

    else if( dispatch_info->m_component == TRELL_COMPONENT_JOB ) {
        if( dispatch_info->m_request == TRELL_REQUEST_POLICY_UPDATE_XML ) {
            apr_hash_t* args = trell_parse_args_uniq_key( r, r->args );
            if( args != NULL ) {
                const char* revision = apr_hash_get( args, "revision", APR_HASH_KEY_STRING );
                if( revision != NULL ) {
                    dispatch_info->m_revision = atoi( revision );
                }
            }
        }
    }

    return OK;
}

