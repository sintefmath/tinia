FILE( GLOB jobobserverSrc "*.cpp" )
FILE( GLOB jobobserverHeaders "../../jobobserver/*.hpp" )

ADD_LIBRARY( jobobserver SHARED ${jobobserverSrc} )

TARGET_LINK_LIBRARIES( jobobserver ${Boost_LIBRARIES} policyLib )

IF(NOT WIN32)
  install( FILES 
    ${jobobserverHeaders} 
    DESTINATION include/jobobserver  
    COMPONENT jobobserver-dev)
  install( TARGETS  
    jobobserver LIBRARY 
    DESTINATION lib/ 
    COMPONENT jobobserver)
ENDIF()
