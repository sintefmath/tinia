SET( LIB_TRELL_SRC
    "IPCGLJobController.cpp"
    "IPCController.cpp"
    "IPCJobController.cpp"
    "OffscreenGL.cpp"
#    "messenger.c"
)

FILE( GLOB LIB_TRELL_HEADERS "../../include/tinia/trell/*.hpp" "../../include/tinia/trell/*.h" )
  
ADD_DEFINITIONS( -g3 -fPIC )
ADD_LIBRARY( tinia_trell ${LIB_TRELL_SRC} ${LIB_TRELL_HEADERS})
TARGET_LINK_LIBRARIES( tinia_trell tiniaipc ${RT} tinia_renderlist ${LIBXML2_LIBRARIES} ${LIB_APR} ${GLEW_LIBRARY} ${OPENGL_LIBRARY} tinia_model tinia_modelxml tinia_jobcontroller)


INSTALL( TARGETS tinia_trell
  EXPORT TiniaTargets
  LIBRARY DESTINATION lib
  COMPONENT trell
  PERMISSIONS WORLD_READ WORLD_EXECUTE OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
)

INSTALL( FILES
  ${LIB_TRELL_HEADERS}
  DESTINATION include/tinia/trell
  COMPONENT trell-dev
)
