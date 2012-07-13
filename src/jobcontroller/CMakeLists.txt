FILE( GLOB jobcontrollerSrc "*.cpp" )
FILE( GLOB jobcontrollerHeaders "../../include/tinia/jobcontroller/*.hpp" )

ADD_LIBRARY( jobcontroller ${jobcontrollerSrc} ${jobcontrollerHeaders} )

TARGET_LINK_LIBRARIES( jobcontroller ${Boost_LIBRARIES} model )

  install( FILES 
    ${jobcontrollerHeaders} 
    DESTINATION include/tinia/jobcontroller  
    COMPONENT jobcontroller_dev)

  install( TARGETS  
    jobcontroller LIBRARY 
    DESTINATION lib/ 
    ARCHIVE DESTINATION lib/
    COMPONENT jobcontroller)
