FILE( GLOB policyLibXmlSrc "*.cpp" )

ADD_LIBRARY( policyLibXml SHARED ${policyLibXmlSrc} )

TARGET_LINK_LIBRARIES( policyLibXml ${Boost_LIBRARIES} ${LIBXML2})