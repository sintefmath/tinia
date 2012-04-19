FILE( GLOB renderlistgl_SRC "*.cpp" )
FILE( GLOB renderlistgl_HEADERS "../../include/librenderlist/gl/*.hpp" )
ADD_LIBRARY( renderlistgl ${renderlistgl_SRC} )
TARGET_LINK_LIBRARIES( renderlistgl
                      ${GLEW_LIBRARY}
                      ${OPENGL_LIBRARIES} )

IF(NOT WIN32)
  install( FILES 
    ${renderlistgl_HEADERS} 
    DESTINATION include/renderlist/gl  
    COMPONENT renderlistgl-dev)
  install( TARGETS  
    renderlistgl ARCHIVE
    DESTINATION lib/ 
    COMPONENT renderlistgl)
ENDIF()