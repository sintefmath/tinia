FILE( GLOB policyLibTestHeaders "*.hpp" )
FILE( GLOB policyLibTestSrc "*.cpp" )

ADD_EXECUTABLE( policylib_unittest ${policyLibTestSrc} ${policyLibTestHeaders} )
TARGET_LINK_LIBRARIES( policylib_unittest ${Boost_LIBRARIES} policyLib )