FILE( GLOB jobobserverSrc "*.cpp" )
FILE( GLOB jobobserverHeaders "../../include/tinia/jobobserver/*.hpp" )

ADD_LIBRARY( jobobserver SHARED ${jobobserverSrc} ${jobobserverHeaders} )

TARGET_LINK_LIBRARIES( jobobserver ${Boost_LIBRARIES} model )

IF(NOT WIN32)
  install( FILES 
    ${jobobserverHeaders} 
    DESTINATION include/tinia/jobobserver  
    COMPONENT jobobserver-dev)
  install( TARGETS  
    jobobserver LIBRARY 
    DESTINATION lib/ 
    COMPONENT jobobserver)
ELSE()
  INSTALL(DIRECTORY include/tinia/jobobserver
    DESTINATION ${tinia_INSTALL_PREFIX}/include/tinia/
    FILES_MATCHING PATTERN "*.hpp"
    PATTERN ".git" EXCLUDE)
  INSTALL(TARGETS jobobserver
    DESTINATION ${tinia_INSTALL_PREFIX}/lib)
ENDIF()
