FILE( GLOB jobcontrollerSrc "*.cpp" )
FILE( GLOB jobcontrollerHeaders "../../include/tinia/jobcontroller/*.hpp" )

ADD_LIBRARY( jobcontroller SHARED ${jobcontrollerSrc} ${jobcontrollerHeaders} )

TARGET_LINK_LIBRARIES( jobcontroller ${Boost_LIBRARIES} model )

IF(NOT WIN32)
  install( FILES 
    ${jobcontrollerHeaders} 
    DESTINATION include/tinia/jobcontroller  
    COMPONENT jobcontroller-dev)
  install( TARGETS  
    jobcontroller LIBRARY 
    DESTINATION lib/ 
    COMPONENT jobcontroller)
ELSE()
  INSTALL(DIRECTORY include/tinia/jobcontroller
    DESTINATION ${tinia_INSTALL_PREFIX}/include/tinia/
    FILES_MATCHING PATTERN "*.hpp"
    PATTERN ".git" EXCLUDE)
  INSTALL(TARGETS jobcontroller
    DESTINATION ${tinia_INSTALL_PREFIX}/lib)
ENDIF()
