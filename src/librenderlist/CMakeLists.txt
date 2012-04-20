FILE( GLOB renderlist_SRC "*.cpp" )
FILE( GLOB renderlist_HEADERS "../../include/tinia/librenderlist/*.hpp")
ADD_LIBRARY( renderlist ${renderlist_SRC} ${renderlist_HEADERS} )
TARGET_LINK_LIBRARIES( renderlist )

IF(NOT WIN32)
  install( FILES 
    ${renderlist_HEADERS} 
    DESTINATION include/tinia/renderlist  
    COMPONENT renderlist-dev)
  install( TARGETS  
    renderlist ARCHIVE
    DESTINATION lib/ 
    COMPONENT renderlist)
ENDIF()
