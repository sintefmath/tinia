FILE( GLOB renderlist_SRC "*.cpp" )
FILE( GLOB renderlist_HEADERS "../../include/tinia/renderlist/*.hpp")

ADD_LIBRARY( renderlist STATIC ${renderlist_SRC} ${renderlist_HEADERS} )
TARGET_LINK_LIBRARIES( renderlist )


# The reason for the development name to be <component>_dev is
# that using <component>-dev does not work with NSIS Windows installer.
# Remember to also update the list of components in the 
# main CMakeLists.txt if you alter component names.

  install( FILES 
    ${renderlist_HEADERS} 
    DESTINATION include/tinia/renderlist  
    COMPONENT renderlist_dev)

  install( TARGETS  
    renderlist
    EXPORT TiniaTargets
    LIBRARY
    DESTINATION lib/ 
    ARCHIVE DESTINATION lib/
    COMPONENT renderlist)

