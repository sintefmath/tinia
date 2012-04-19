FILE( GLOB renderlist_SRC "*.cpp" )
FILE( GLOB renderlist_HEADERS "../../include/librenderlist/*.hpp")
ADD_LIBRARY( renderlist ${renderlist_SRC} )
TARGET_LINK_LIBRARIES( renderlist )

IF(NOT WIN32)
  install( FILES 
    ${renderlist_HEADERS} 
    DESTINATION include/renderlist  
    COMPONENT renderlist-dev)
  install( TARGETS  
    renderlist ARCHIVE
    DESTINATION lib/ 
    COMPONENT renderlist)
ENDIF()
