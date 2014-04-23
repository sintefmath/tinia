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

find_package (Threads)
ADD_EXECUTABLE( tinia_trell_master ${TRELL_MASTER_SRC} )
TARGET_LINK_LIBRARIES( tinia_trell_master tinia_renderlist tinia_renderlistgl tinia_trell ${RT} ${CMAKE_THREAD_LIBS_INIT} tinia_jobcontroller  ${LIBXML2_LIBRARIES} ${OPENGL_LIBRARY} tinia_model tinia_modelxml  )


INSTALL( TARGETS 
  tinia_trell_master
  EXPORT TiniaTargets
  RUNTIME DESTINATION var/trell/bin
    COMPONENT mod_trell
    PERMISSIONS WORLD_READ WORLD_EXECUTE OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
)

