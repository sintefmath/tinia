FILE( GLOB modelxmlSrc "*.cpp" )
FILE( GLOB modelxmlHeaders "../../include/tinia/impl/xml/*.hpp" )

ADD_LIBRARY( modelxml ${modelxmlSrc} ${modelxmlHeaders})

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
