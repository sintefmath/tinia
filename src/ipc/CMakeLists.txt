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

  install( DIRECTORY
    "../../include/tinia/ipc"
    DESTINATION include/tinia/
    COMPONENT tiniaipc)

  install( TARGETS
    tiniaipc
    EXPORT TiniaTargets
    LIBRARY
    DESTINATION lib/ 
    ARCHIVE DESTINATION lib/
    COMPONENT tiniaipc )
    
