SET( LIB_TRELL_SRC
    "IPCGLJobObserver.cpp"
    "IPCObserver.cpp"
    "IPCJobObserver.cpp"
    "messenger.c"
)

FILE( GLOB LIB_TRELL_HEADERS "../../include/trell/*.hpp" "../../include/trell/*.h" )
  

ADD_LIBRARY( trell SHARED ${LIB_TRELL_SRC} )
TARGET_LINK_LIBRARIES( trell ${RT} renderlist ${LIBXML2_LIBRARIES} ${LIB_APR} ${GLEW_LIBRARY} ${OPENGL_LIBRARY} policyLib policyLibXml jobobserver)

INSTALL( TARGETS trell
  LIBRARY DESTINATION lib
  COMPONENT libtrell

)

INSTALL( FILES
  ${LIB_TRELL_HEADERS}
  DESTINATION include/trell
  COMPONENT libtrell-dev
)