FILE( GLOB modelxmlSrc "*.cpp" )
FILE( GLOB modelxmlHeaders "../../include/tinia/impl/xml/*.hpp" )

SET( CPACK_GENERATOR "DEB" )
SET( CPACK_PACKAGE_NAME "modelxml" )
SET( CPACK_DEBIAN_PACKAGE_NAME "modelxml" )
SET( CPACK_PACKAGE_VERSION ${version_number} )
SET( CPACK_DEBIAN_PACKAGE_MAINTAINER "kjetil.olsen.lye@sintef.no" )
SET( CPACK_COMPONENTS_ALL libraries headers )
SET( CPACK_COMPONENT_HEADERS_DEPENDS libraries )
SET( CPACK_PACKAGE_INSTALL_DIRECTORY "/" )
include ( CPack )

ADD_LIBRARY( modelxml SHARED ${modelxmlSrc} ${modelxmlHeaders})

TARGET_LINK_LIBRARIES( modelxml ${Boost_LIBRARIES} ${LIBXML2_LIBRARIES})

IF(NOT WIN32)
  install( FILES 
    ${modelxmlHeaders} 
    DESTINATION include/tinia/modelxml
    COMPONENT modelxml-dev)
  install( TARGETS  
    modelxml LIBRARY 
    DESTINATION lib/ 
    COMPONENT modelxml)
ENDIF()
