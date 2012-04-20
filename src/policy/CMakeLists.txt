FILE( GLOB policySrc "*.cpp" )
FILE( GLOB policyHeaders "../../include/tinia/policy/*.hpp" )


ADD_LIBRARY( policy SHARED ${policySrc} ${policyHeaders} )

TARGET_LINK_LIBRARIES( policy ${Boost_LIBRARIES} )




IF(NOT WIN32)
  install( FILES 
    ${policyHeaders} 
    DESTINATION include/tinia/policy  
    COMPONENT policy-dev)
  install( TARGETS  
    policy LIBRARY 
    DESTINATION lib/ 
    COMPONENT policy)
ENDIF()
