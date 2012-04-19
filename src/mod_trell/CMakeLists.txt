SET( MOD_TRELL_SRC
    "mod_trell.c"
    "mod_trell_ops.c"
    "mod_trell_job.c"
    "mod_trell_util.c"
    "mod_trell_reply.c"
    "mod_trell_send_png.c"
    "mod_trell_send_bmp.c"
    "mod_trell_send_xml.c"
    "mod_trell_send_file.c"
    "mod_trell_url_decode.c"
    "../lib_trell/messenger.c"
)

ADD_LIBRARY( mod_trell SHARED ${MOD_TRELL_SRC} )
TARGET_LINK_LIBRARIES( mod_trell ${RT} ${LIBXML2_LIBRARIES} ${LIB_APR} )