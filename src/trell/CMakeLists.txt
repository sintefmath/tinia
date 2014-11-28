IF( TINIA_SERVER)
SET( LIB_TRELL_SRC
    "IPCGLJobController.cpp"
    "IPCController.cpp"
    "IPCJobController.cpp"
    "OffscreenGL.cpp"
#    "messenger.c"
)

FILE( GLOB LIB_TRELL_HEADERS "../../include/tinia/trell/*.hpp" "../../include/tinia/trell/*.h" )
  
FIND_PACKAGE(Threads)
ADD_LIBRARY( tinia_trell ${LIB_TRELL_SRC} ${LIB_TRELL_HEADERS})
TARGET_LINK_LIBRARIES( tinia_trell tiniaipc ${RT} ${CMAKE_THREAD_LIBS_INIT} tinia_renderlist ${LIBXML2_LIBRARIES} ${LIB_APR} ${GLEW_LIBRARY} ${OPENGL_LIBRARY} tinia_model tinia_modelxml tinia_jobcontroller)

INSTALL( TARGETS tinia_trell
  EXPORT TiniaTargets
  LIBRARY DESTINATION lib
  COMPONENT libtrell
  PERMISSIONS WORLD_READ WORLD_EXECUTE OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
)

INSTALL( FILES
  ${LIB_TRELL_HEADERS}
  DESTINATION include/tinia/trell
  COMPONENT libtrell-dev
)
ENDIF( TINIA_SERVER )

FIND_PACKAGE( Qt5Core )
FIND_PACKAGE( Qt5WebSockets )
FIND_PACKAGE( Freeglut REQUIRED )
INCLUDE_DIRECTORIES( ${Freeglut_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR})
FILE( GLOB FRV_HEADERS "../../include/tinia/trell/FRV*.hpp" "../../include/tinia/jobcontroller/Controller.hpp")
FILE( GLOB FRV_SRC "FRV*.cpp" )
MESSAGE( "SRC: ${FRV_SRC}")
MESSAGE( "HDR: ${FRV_HEADERS}")
ADD_LIBRARY( tinia_frvController ${FRV_HEADERS} ${FRV_SRC})
TARGET_LINK_LIBRARIES( tinia_frvController ${GLEW_LIBRARY} tinia_jobcontroller tinia_model ${Freeglut_LIBRARY})
QT5_USE_MODULES( tinia_frvController WebSockets )

