FILE( GLOB jobcontrollerSrc "*.cpp" )
FILE( GLOB jobcontrollerHeaders "../../include/tinia/jobcontroller/*.hpp" )

ADD_LIBRARY( tinia_jobcontroller ${jobcontrollerSrc} ${jobcontrollerHeaders} )

TARGET_LINK_LIBRARIES( tinia_jobcontroller ${Boost_LIBRARIES} tinia_model )


# The reason for the development name to be <component>_dev is
# that using <component>-dev does not work with NSIS Windows installer.
# Remember to also update the list of components in the 
# main CMakeLists.txt if you alter component names.
  install( DIRECTORY 
    "../../include/tinia/jobcontroller" 
    DESTINATION include/tinia/
    COMPONENT jobcontroller_dev)

  install( TARGETS  
    tinia_jobcontroller LIBRARY
    DESTINATION lib/ 
    ARCHIVE DESTINATION lib/
    COMPONENT jobcontroller)
