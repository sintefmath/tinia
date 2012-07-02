FILE( GLOB renderlistgl_SRC "*.cpp" )
FILE( GLOB renderlistgl_HEADERS "../../include/tinia/renderlist/gl/*.hpp" )
ADD_LIBRARY( renderlistgl STATIC ${renderlistgl_SRC} ${renderlistgl_HEADERS} )
TARGET_LINK_LIBRARIES( renderlistgl
                      ${GLEW_LIBRARY}
                      ${OPENGL_LIBRARIES} )

IF(NOT WIN32)
  install( FILES 
    ${renderlistgl_HEADERS} 
    DESTINATION include/tinia/renderlist/gl  
    COMPONENT renderlistgl-dev)
  install( TARGETS  
    renderlistgl ARCHIVE
    DESTINATION lib/ 
    COMPONENT renderlistgl)
ELSE()
 INSTALL(DIRECTORY include/tinia/renderlist/gl
    DESTINATION ${tinia_INSTALL_PREFIX}/include/tinia/renderlist/
    FILES_MATCHING PATTERN "*.hpp"
    PATTERN ".git" EXCLUDE)
  INSTALL(TARGETS renderlistgl
    DESTINATION ${tinia_INSTALL_PREFIX}/lib)
ENDIF()
