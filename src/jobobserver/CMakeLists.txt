FILE( GLOB jobobserverSrc "*.cpp" )
FILE( GLOB jobobserverHeaders "../../include/tinia/jobobserver/*.hpp" )

ADD_LIBRARY( jobobserver SHARED ${jobobserverSrc} ${jobobserverHeaders} )

TARGET_LINK_LIBRARIES( jobobserver ${Boost_LIBRARIES} policy )

IF(NOT WIN32)
  install( FILES 
    ${jobobserverHeaders} 
    DESTINATION include/tinia/jobobserver  
    COMPONENT jobobserver-dev)
  install( TARGETS  
    jobobserver LIBRARY 
    DESTINATION lib/ 
    COMPONENT jobobserver)
ENDIF()
