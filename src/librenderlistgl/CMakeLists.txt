FILE( GLOB renderlistgl_SRC "*.cpp" )
ADD_LIBRARY( renderlistgl ${renderlistgl_SRC} )
TARGET_LINK_LIBRARIES( renderlistgl
                      ${GLEW_LIBRARY}
                      ${OPENGL_LIBRARIES} )