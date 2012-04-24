FILE( GLOB renderlist_SRC "*.cpp" )
FILE( GLOB renderlist_HEADERS "../../include/tinia/librenderlist/*.hpp")
ADD_LIBRARY( renderlist ${renderlist_SRC} ${renderlist_HEADERS} )
TARGET_LINK_LIBRARIES( renderlist )

IF(NOT WIN32)
  install( FILES 
    ${renderlist_HEADERS} 
    DESTINATION include/tinia/librenderlist  
    COMPONENT renderlist-dev)
  install( TARGETS  
    renderlist ARCHIVE
    DESTINATION lib/ 
    COMPONENT renderlist)
ELSE()
 INSTALL(DIRECTORY include/tinia/librenderlist
    DESTINATION ${tinia_INSTALL_PREFIX}/include/tinia/
    FILES_MATCHING PATTERN "*.hpp"
    PATTERN ".git" EXCLUDE)
  INSTALL(TARGETS renderlist
    DESTINATION ${tinia_INSTALL_PREFIX}/lib)
ENDIF()
