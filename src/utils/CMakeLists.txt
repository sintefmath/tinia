FILE( GLOB utilsSrc "*.cpp" )
FILE( GLOB_RECURSE utilsHeaders "../../include/tinia/utils/*.hpp")


ADD_LIBRARY( tinia_utils ${utilsSrc} ${utilsHeaders} )

TARGET_LINK_LIBRARIES( tinia_utils ${Boost_LIBRARIES} )

# The reason for the development name to be <component>_dev is
# that using <component>-dev does not work with NSIS Windows installer.
# Remember to also update the list of components in the 
# main CMakeLists.txt if you alter component names.

  install( DIRECTORY
    "../../include/tinia/utils" 
    DESTINATION include/tinia/  
    COMPONENT utils_dev)

  install( TARGETS
    tinia_utils
    EXPORT TiniaTargets
    LIBRARY
    DESTINATION lib/ 
    ARCHIVE DESTINATION lib/
    COMPONENT utils )
    
