SET( LIB_IPC_SRC
    "ipc_msg_client.c"
    "ipc_msg_common.c"
    "ipc_msg_server.c"
    "ipc_util.c"
)

FILE( GLOB LIB_IPC_HEADERS "ipc_msg_internal.h" "../../include/tinia/ipc/*.h" )
  
ADD_DEFINITIONS(-fPIC )

ADD_LIBRARY( tiniaipc STATIC ${LIB_IPC_SRC} ${LIB_IPC_HEADERS})
TARGET_LINK_LIBRARIES( tiniaipc ${RT} )


INSTALL( TARGETS tiniaipc
  EXPORT TiniaTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  COMPONENT trell
)

#INSTALL( TARGETS trell
#  LIBRARY DESTINATION lib
#  COMPONENT libtrell
#  PERMISSIONS WORLD_READ WORLD_EXECUTE OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
#)

#INSTALL( FILES
#  ${LIB_TRELL_HEADERS}
#  DESTINATION include/tinia/trell
#  COMPONENT libtrell-dev
#)
