SET( LIB_TRELL_SRC
    "IPCGLJobController.cpp"
    "IPCController.cpp"
    "IPCJobController.cpp"
    "messenger.c"
)

FILE( GLOB LIB_TRELL_HEADERS "../../include/tinia/trell/*.hpp" "../../include/tinia/trell/*.h" )
  

ADD_LIBRARY( tinia_trell ${LIB_TRELL_SRC} ${LIB_TRELL_HEADERS})
TARGET_LINK_LIBRARIES( tinia_trell ${RT} tinia_renderlist ${LIBXML2_LIBRARIES} ${LIB_APR} ${GLEW_LIBRARY} ${OPENGL_LIBRARY} tinia_model tinia_modelxml tinia_jobcontroller)

INSTALL( TARGETS tinia_trell
  LIBRARY DESTINATION lib
  COMPONENT libtrell
  PERMISSIONS WORLD_READ WORLD_EXECUTE OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
)

INSTALL( FILES
  ${LIB_TRELL_HEADERS}
  DESTINATION include/tinia/trell
  COMPONENT libtrell-dev
)
