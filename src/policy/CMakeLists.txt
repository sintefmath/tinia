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
ELSE()
 INSTALL(DIRECTORY include/tinia/policy
    DESTINATION ${tinia_INSTALL_PREFIX}/include/tinia/
    FILES_MATCHING PATTERN "*.hpp"
    PATTERN ".git" EXCLUDE)
  INSTALL(TARGETS policy
    DESTINATION ${tinia_INSTALL_PREFIX}/lib)
ENDIF()
