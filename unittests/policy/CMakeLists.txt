FILE( GLOB policyTestHeaders "*.hpp" )
FILE( GLOB policyTestSrc "*.cpp" )

ADD_EXECUTABLE( policy_unittest ${policyTestSrc} ${policyTestHeaders} )
TARGET_LINK_LIBRARIES( policy_unittest ${Boost_LIBRARIES} policy )