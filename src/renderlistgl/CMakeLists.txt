FILE( GLOB renderlistgl_SRC "*.cpp" )
FILE( GLOB renderlistgl_HEADERS "../../include/tinia/renderlist/gl/*.hpp" )
find_package( GLEW REQUIRED )
set(GLEW_DEFINITIONS -DGLEW_STATIC)
ADD_LIBRARY( tinia_renderlistgl STATIC ${renderlistgl_SRC} ${renderlistgl_HEADERS} )
set(GLEW_DEFINITIONS -DGLEW_STATIC)
TARGET_LINK_LIBRARIES( tinia_renderlistgl
                      ${GLEW_LIBRARIES}
                      ${OPENGL_LIBRARIES} )


# The reason for the development name to be <component>_dev is
# that using <component>-dev does not work with NSIS Windows installer.
# Remember to also update the list of components in the 
# main CMakeLists.txt if you alter component names.

  install( FILES 
    ${renderlistgl_HEADERS} 
    DESTINATION include/tinia/renderlist/gl  
    COMPONENT renderlistgl_dev)

  install( TARGETS  
    tinia_renderlistgl
    EXPORT TiniaTargets
    LIBRARY
    DESTINATION lib/ 
    ARCHIVE DESTINATION lib/
    COMPONENT renderlistgl)
