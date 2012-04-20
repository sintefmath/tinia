FILE( GLOB policyLibSrc "*.cpp" )
FILE( GLOB policyLibHeaders "../../include/tinia/policylib/*.hpp" )


ADD_LIBRARY( policyLib SHARED ${policyLibSrc} ${policyLibHeaders} )

TARGET_LINK_LIBRARIES( policyLib ${Boost_LIBRARIES} )




IF(NOT WIN32)
  install( FILES 
    ${policyLibHeaders} 
    DESTINATION include/tinia/policy  
    COMPONENT policylib-dev)
  install( TARGETS  
    policyLib LIBRARY 
    DESTINATION lib/ 
    COMPONENT policylib)
ENDIF()
