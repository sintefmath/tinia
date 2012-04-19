FILE( GLOB policyLibSrc "*.cpp" )

ADD_LIBRARY( policyLib SHARED ${policyLibSrc} )

TARGET_LINK_LIBRARIES( policyLib ${Boost_LIBRARIES} )
