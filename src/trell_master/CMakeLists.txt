SET( TRELL_MASTER_SRC
    "master_main.cpp"
    "Master.cpp"
    "Application.cpp"
    "Application.hpp"
    "Applications.cpp"
    "Applications.hpp"
    "RenderingDevices.hpp"
    "RenderingDevices.cpp"
)

ADD_EXECUTABLE( trell_master ${TRELL_MASTER_SRC} )
TARGET_LINK_LIBRARIES( trell_master renderlist renderlistgl trell ${RT} jobcontroller  ${LIBXML2_LIBRARIES} ${OPENGL_LIBRARY} model modelxml )


INSTALL( TARGETS 
  trell_master
  EXPORT TiniaTargets
  RUNTIME DESTINATION var/trell/bin
    COMPONENT mod_trell
    PERMISSIONS WORLD_READ WORLD_EXECUTE OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
)

