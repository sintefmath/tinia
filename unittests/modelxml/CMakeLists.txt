FILE( GLOB policyxmlTestHeaders "*.hpp" )
FILE( GLOB policyxmlTestSrc "*.cpp" )

ADD_EXECUTABLE( policyxml_unittest ${policyxmlTestSrc} ${policyxmlTestHeaders} )
TARGET_LINK_LIBRARIES( policyxml_unittest ${Boost_LIBRARIES} ${LIBXML2_LIBRARIES} policy policyxml )