SET( LIB_TRELL_SRC
    "IPCGLJobObserver.cpp"
    "IPCObserver.cpp"
    "IPCJobObserver.cpp"
    "messenger.c"
)

ADD_LIBRARY( trell SHARED ${LIB_TRELL_SRC} )
TARGET_LINK_LIBRARIES( trell ${RT} renderlist ${LIBXML2_LIBRARIES} ${LIB_APR} ${GLEW_LIBRARY} ${OPENGL_LIBRARY} policyLib policyLibXml jobobserver)