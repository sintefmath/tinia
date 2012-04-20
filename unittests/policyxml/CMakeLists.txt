FILE( GLOB policyXmlTestHeaders "*.hpp" )
FILE( GLOB policyXmlTestSrc "*.cpp" )

ADD_EXECUTABLE( policyxml_unittest ${policyXmlTestSrc} ${policyXmlTestHeaders} )
TARGET_LINK_LIBRARIES( policyxml_unittest ${Boost_LIBRARIES} ${LIBXML2_LIBRARIES} policy policyXml )