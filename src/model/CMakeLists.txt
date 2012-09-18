FILE( GLOB modelSrc "*.cpp" )
FILE( GLOB_RECURSE modelHeaders "../../include/tinia/model/*.hpp")


ADD_LIBRARY( model ${modelSrc} ${modelHeaders} )

TARGET_LINK_LIBRARIES( model ${Boost_LIBRARIES} )

# The reason for the development name to be <component>_dev is
# that using <component>-dev does not work with NSIS Windows installer.
# Remember to also update the list of components in the 
# main CMakeLists.txt if you alter component names.

  install( DIRECTORY
    "../../include/tinia/model" 
    DESTINATION include/tinia/  
    COMPONENT model_dev)

  install( TARGETS  
    model LIBRARY 
    DESTINATION lib/ 
    ARCHIVE DESTINATION lib/
    COMPONENT model )
    
