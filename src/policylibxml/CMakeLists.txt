FILE( GLOB policyLibXmlSrc "*.cpp" )
FILE( GLOB policylibXmlHeaders "../../include/policylibxml/*.hpp" )

SET( CPACK_GENERATOR "DEB" )
SET( CPACK_PACKAGE_NAME "policylibxml" )
SET( CPACK_DEBIAN_PACKAGE_NAME "policylibxml" )
SET( CPACK_PACKAGE_VERSION ${version_number} )
SET( CPACK_DEBIAN_PACKAGE_MAINTAINER "kjetil.olsen.lye@sintef.no" )
SET( CPACK_COMPONENTS_ALL libraries headers )
SET( CPACK_COMPONENT_HEADERS_DEPENDS libraries )
SET( CPACK_PACKAGE_INSTALL_DIRECTORY "/" )
include ( CPack )

ADD_LIBRARY( policyLibXml SHARED ${policyLibXmlSrc} )

TARGET_LINK_LIBRARIES( policyLibXml ${Boost_LIBRARIES} ${LIBXML2_LIBRARIES})

IF(NOT WIN32)
  install( FILES 
    ${policyLibXmlHeaders} 
    DESTINATION include/policylibxml
    COMPONENT policylibxml-dev)
  install( TARGETS  
    policyLibXml LIBRARY 
    DESTINATION lib/ 
    COMPONENT policylibxml)
ENDIF()
