FILE( GLOB policyLibXmlTestHeaders "*.hpp" )
FILE( GLOB policyLibXmlTestSrc "*.cpp" )

ADD_EXECUTABLE( policylibxml_unittest ${policyLibXmlTestSrc} ${policyLibXmlTestHeaders} )
TARGET_LINK_LIBRARIES( policylibxml_unittest ${Boost_LIBRARIES} ${LIBXML2_LIBRARIES} policyLib policyLibXml )