FILE( GLOB policyXmlSrc "*.cpp" )
FILE( GLOB policyXmlHeaders "../../include/tinia/policyxml/*.hpp" )

SET( CPACK_GENERATOR "DEB" )
SET( CPACK_PACKAGE_NAME "policyxml" )
SET( CPACK_DEBIAN_PACKAGE_NAME "policyxml" )
SET( CPACK_PACKAGE_VERSION ${version_number} )
SET( CPACK_DEBIAN_PACKAGE_MAINTAINER "kjetil.olsen.lye@sintef.no" )
SET( CPACK_COMPONENTS_ALL libraries headers )
SET( CPACK_COMPONENT_HEADERS_DEPENDS libraries )
SET( CPACK_PACKAGE_INSTALL_DIRECTORY "/" )
include ( CPack )

ADD_LIBRARY( policyXml SHARED ${policyXmlSrc} ${policyXmlHeaders})

TARGET_LINK_LIBRARIES( policyXml ${Boost_LIBRARIES} ${LIBXML2_LIBRARIES})

IF(NOT WIN32)
  install( FILES 
    ${policyXmlHeaders} 
    DESTINATION include/tinia/policyxml
    COMPONENT policyxml-dev)
  install( TARGETS  
    policyXml LIBRARY 
    DESTINATION lib/ 
    COMPONENT policyxml)
ENDIF()
