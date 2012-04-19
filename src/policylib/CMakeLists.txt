FILE( GLOB policyLibSrc "*.cpp" )
FILE( GLOB policyLibHeaders "../../include/*.hpp" )


ADD_LIBRARY( policyLib SHARED ${policyLibSrc} )

TARGET_LINK_LIBRARIES( policyLib ${Boost_LIBRARIES} )




IF(NOT WIN32)
  install( FILES 
    ${policyLibHeaders} 
    DESTINATION include/policylib  
    COMPONENT policylib-dev)
  install( TARGETS  
    policyLib LIBRARY 
    DESTINATION lib/ 
    COMPONENT policylib)
ENDIF()
