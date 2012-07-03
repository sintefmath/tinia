FILE( GLOB renderlist_SRC "*.cpp" )
FILE( GLOB renderlist_HEADERS "../../include/tinia/renderlist/*.hpp")

ADD_LIBRARY( renderlist STATIC ${renderlist_SRC} ${renderlist_HEADERS} )
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
ELSE()
 INSTALL(DIRECTORY include/tinia/renderlist
    DESTINATION include/tinia/
    FILES_MATCHING PATTERN "*.hpp"
    PATTERN ".git" EXCLUDE)
  INSTALL(TARGETS renderlist
    DESTINATION lib)
ENDIF()
