SET( MOD_TRELL_SRC
    "mod_trell.c"
    "filter_validate_xml.c"
    "mod_trell_ops.c"
    "mod_trell_job.c"
    "mod_trell_send_file.c"
    "mod_trell_url_decode.c"
    "pass_query.c"
    "pass_reply.c"
    "pass_reply_png.c"
    "pass_reply_jpg.c"
)

FIND_PACKAGE(Threads)
ADD_LIBRARY( mod_tinia_trell ${MOD_TRELL_SRC} )
TARGET_LINK_LIBRARIES( mod_tinia_trell tiniaipc ${RT} ${CMAKE_THREAD_LIBS_INIT} ${LIBXML2_LIBRARIES} ${LIB_APR} ${LIB_TURBOJPEG})
ADD_DEFINITIONS( -Wall )

INSTALL( TARGETS mod_tinia_trell
  EXPORT TiniaTargets
  LIBRARY DESTINATION var/trell/module
  PERMISSIONS WORLD_READ WORLD_EXECUTE OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
  COMPONENT mod_trell
)

INSTALL( FILES ../../config/mod_trell.load ../../config/mod_trell.conf
  DESTINATION share/tinia/conf
  PERMISSIONS WORLD_READ OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
  COMPONENT mod_trell
)

# Note that mod_trell.conf is the same for both Debug and Release, mod_trell.load differs.
# Note also that the one for Release below is just a copy of the default one above.

INSTALL( FILES ../../config/dbg/mod_trell.load ../../config/mod_trell.conf
  DESTINATION share/tinia/conf
  PERMISSIONS WORLD_READ OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
  CONFIGURATIONS Debug
  COMPONENT mod_trell
)

INSTALL( FILES ../../config/mod_trell.load ../../config/mod_trell.conf
  DESTINATION share/tinia/conf
  PERMISSIONS WORLD_READ OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
  CONFIGURATIONS Release
  COMPONENT mod_trell
)

INSTALL( DIRECTORY "../../js"
  DESTINATION var/trell
  COMPONENT mod_trell
  PATTERN ".git" EXCLUDE
)


INSTALL( DIRECTORY ../../static
  DESTINATION var/trell/
  COMPONENT mod_trell
  PATTERN ".git" EXCLUDE
)

INSTALL( DIRECTORY ../../schemas
  DESTINATION var/trell/
  COMPONENT mod_trell
  PATTERN ".git" EXCLUDE
)
